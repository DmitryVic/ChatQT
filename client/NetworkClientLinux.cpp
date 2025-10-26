// Реализация NetworkClient для Linux
#include <string>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <unistd.h>             // базовые функции для работы с системой Linux
#include <sys/socket.h>         // для работы с сокетами
#include <netinet/in.h>         // содержит структуры и константы для работы с протоколами
#include <stdexcept>            // исключения
#include <arpa/inet.h>          // преобразовать ip inet_pton()
#include <sys/select.h>         // для select()
#include "NetworkClient.h"
#include "UserStatus.h"
#include "Logger.h"


// Конструктор: принимает shared_ptr<UserStatus>
NetworkClient::NetworkClient(const std::string& ip, int port, std::shared_ptr<UserStatus> status)
    : server_ip(ip), port(port), status(std::move(status)), sock(-1) {}  // -1 = сокет не инициализирован

        
NetworkClient::~NetworkClient() {
    stopThreads();
    if (sock != -1) {
        close(sock);
        get_logger() << "Соединение с сервером закрыто";
    }
}
// Запуск потоков приёма и отправки
void NetworkClient::startThreads() {
    if (threadsRunning.load()) return;
    threadsRunning.store(true);
    recvThread = std::thread(&NetworkClient::recvLoop, this);
    sendThread = std::thread(&NetworkClient::sendLoop, this);
    status->setNetworckThreadsSost(true);
    get_logger() << "Потоки запущены";
}

// Остановка потоков приёма и отправки
void NetworkClient::stopThreads() {
    if (!threadsRunning.load()) return;
    threadsRunning.store(false);
    if (recvThread.joinable()) recvThread.join();
    if (sendThread.joinable()) sendThread.join();
    status->setNetworckThreadsSost(false);
    get_logger() << "Потоки остановлены";
}

// Функция для потока приёма
void NetworkClient::recvLoop() {
    while (threadsRunning.load() && status && status->running() && status->getNetworckConnect()) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500 мс

        // Ждем доступность сокета для чтения с таймаутом
        int sel = select(sock + 1, &readfds, nullptr, nullptr, &timeout);
        // sel > 0 - сокет готов к чтению
        // sel == 0 - таймаут
        if (sel > 0 && FD_ISSET(sock, &readfds)) {
            try {
                std::string msg = getMess();
                
                if (msg == "") {
                    // Пауза на 3 секунды
                    // std::this_thread::sleep_for(std::chrono::seconds(3));
                    // ПЕРЕПРОВЕРКА подключания и заного подключится 
                    // пока просто выход
                    status->setNetworckConnect(false);
                    break;
                }
                
                // если соединение оборвано,, то getMess будет постоянно отдавать ""
                // НУЖНО ДОБАВИТЬ ОСТАНОВКУ И ПЕРЕПРОВЕРКУ В ЦИКЛЕ 
                
                if (!msg.empty()) {
                    status->pushAcceptedMessage(msg);
                }
            } catch (const std::exception& e) {
                get_logger() << "Ошибка приёма: " << e.what();
                break;
            }
        } else if (sel == 0) {
            // timeout, сообщение не пришло
            // пропускаем для  того чтобы перепроверить флаг работоспособности
            continue;
        } else if (sel < 0) {
            get_logger() << "Ошибка select() в recvLoop";
            break;
        }
    }
}

// Функция для потока отправки
void NetworkClient::sendLoop() {
    while (threadsRunning.load() && status && status->running() && status->getNetworckConnect()) {
        std::string msg = status->waitAndPopMessageToSend();
        if (!msg.empty()) {
            try {
                sendMess(msg);
            } catch (const std::exception& e) {
                get_logger() << "Ошибка отправки: " << e.what();
                break;
            }
        }
    }
}

// Метод подключения к серверу
void NetworkClient::connecting() {
    bool err = false;
    // СОЗДАНИЕ СОКЕТА
    // AF_INET = IPv4, SOCK_STREAM = TCP, 0 = протокол по умолчанию
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        status->setNetworckConnect(false);
        err = true;
        get_logger() << "Не удалось создать сокет";
        throw std::runtime_error("Не удалось создать сокет");
    }

    // НАСТРОЙКА АДРЕСА СЕРВЕРА
    sockaddr_in serv_addr;               // Структура для адреса сервера
    serv_addr.sin_family = AF_INET;      // Семейство адресов - IPv4
    
    // Преобразование порта в сетевой порядок байт (big-endian)
    serv_addr.sin_port = htons(port);    
    
    // Преобразование IP-адреса из строки в бинарный формат
    // inet_pton (IP presentation to network)
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        close(sock);  // Закрываем сокет при ошибке
        //throw std::runtime_error("Некорректный адрес сервера");
        get_logger() << "Некорректный адрес сервера";
        status->setNetworckConnect(false);
        err = true;
    }

    // ПОДКЛЮЧЕНИЕ К СЕРВЕРУ
    // connect() устанавливает соединение с указанным адресом
    // sizeof(serv_addr) - размер структуры адреса
    get_logger() << "Подключение NETWORK\n";    
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr))) {
        close(sock);
        // throw std::runtime_error("Ошибка подключения");
        get_logger() << "Ошибка подключения";
        status->setNetworckConnect(false);
        err = true;
    }

    if (!err)
    {
        status->setNetworckConnect(true);
    }
    
}

// ОТПРАВКА СООБЩЕНИЯ
void NetworkClient::sendMess(const std::string& message) {
    // send() записывает данные в сокет
    // message.c_str() - указатель на данные
    // message.size() - длина данных в байтах
    // 0 - флаги (по умолчанию)
    if (send(sock, message.c_str(), message.size(), 0) < 0) {
        get_logger() << "Ошибка отправки";
        status->setNetworckConnect(false);
        // throw std::runtime_error("Ошибка отправки");
    }
}


std::string NetworkClient::getMess() {
    const size_t TMP_BUF = 64 * 1024; // читаем по 64KB
    char tmp[TMP_BUF];

    // вернёт {start, end} (inclusive) или {npos,npos}
    auto extract_one_json_range = [&](const std::string &buf) -> std::pair<size_t, size_t> {
        size_t n = buf.size();
        size_t i = 0;
        while (i < n && isspace(static_cast<unsigned char>(buf[i]))) ++i;
        // найти '{' или '['
        while (i < n && buf[i] != '{' && buf[i] != '[') ++i;
        if (i >= n) return {std::string::npos, std::string::npos}; // std::string::npos  - специальное значение, означающее "не найдено"

        char open_ch = buf[i];
        char close_ch = (open_ch == '{') ? '}' : ']';

        bool in_str = false;
        bool esc = false;
        int depth = 0;

        for (size_t p = i; p < n; ++p) {
            char c = buf[p];
            if (!in_str) {
                if (c == open_ch) ++depth;
                else if (c == close_ch) {
                    --depth;
                    if (depth == 0) return {i, p};
                } else if (c == '"') {
                    in_str = true;
                    esc = false;
                }
            } else {
                if (esc) esc = false;
                else {
                    if (c == '\\') esc = true;
                    else if (c == '"') in_str = false;
                }
            }
        }
        return {std::string::npos, std::string::npos}; // std::string::npos  - специальное значение, означающее "не найдено"
    };

    // 1) если в буфере уже есть сообщение — вернём сразу
    auto r = extract_one_json_range(recv_buffer_);
    if (r.first != std::string::npos) { // есть полный JSON
        size_t start = r.first, end = r.second;
        std::string msg = recv_buffer_.substr(start, end - start + 1);
        recv_buffer_.erase(0, end + 1);
        return msg;
    }

    // 2) читаем из сокета пока не соберём полный JSON либо не произойдёт ошибка/закрытие
    while (threadsRunning.load() && status && status->running() && status->getNetworckConnect()) {
        ssize_t n = ::recv(sock, tmp, sizeof(tmp), 0);      // читаем из сокета
        if (n > 0) {                                        // получили n байт
            recv_buffer_.append(tmp, static_cast<size_t>(n));

            const size_t MAX_ALLOWED = 32 * 1024 * 1024;    // 32 MB
            if (recv_buffer_.size() > MAX_ALLOWED) {        // защитный лимит
                get_logger() << "Получено слишком большое сообщение: " << recv_buffer_.size() << " байт";
                status->setNetworckConnect(false);
                throw std::runtime_error("Сообщение превышает допустимый размер");
            }

            r = extract_one_json_range(recv_buffer_);       // пробуем извлечь JSON
            if (r.first != std::string::npos) {             // есть полный JSON
                size_t start = r.first, end = r.second;
                std::string msg = recv_buffer_.substr(start, end - start + 1);
                recv_buffer_.erase(0, end + 1);
                return msg;
            }

            // иначе — продолжим читать (recv будет блокировать, пока не появятся данные)
            continue;
        } else if (n == 0) {
            get_logger() << "getMess | Сервер закрыл соединение\n";
            status->setNetworckConnect(false);
            return "";
        } else { // n < 0
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // если сокет неблокирующий — можно подождать или просто продолжить
                continue;
            } else {
                get_logger() << "recv error: " << strerror(errno);
                status->setNetworckConnect(false);
                throw std::runtime_error("Ошибка чтения сокета");
            }
        }
    } // while

    // если флаги работы изменились — выходим
    return "";
}
