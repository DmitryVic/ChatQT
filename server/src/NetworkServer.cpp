#include "NetworkServer.h"
#include <string>
#include <iostream>
#include <unistd.h>             //базовые функции для работы с системой Linux
#include <sys/socket.h>         //для работы с сокетами
#include <netinet/in.h>         //содержит структуры и константы для работы с протоколами
#include <stdexcept>            //исключения
#include "dataUserOnline.h"
#include "Logger.h"
#include <cerrno>               // для errno
#include <cstring>              // для strerror
#include <sys/select.h>         // для select()


void NetworkServer::start() {
    //создает сокет ТСП
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("Сокет _server_fd не создан!");
    
    // Настройка конфигурации
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //привязать сокет к порту
    if (bind(_server_fd, (sockaddr*)&server_addr, sizeof(server_addr))) {
        close(_server_fd);
        throw std::runtime_error("Связь потеряна!");
    }

    //прослушиваем сокет (максимально подключений 5)
    if (listen(_server_fd, 5)) {
        close(_server_fd);
        throw  std::runtime_error("Ошибка при прослушивании сокета!");
    }

    get_logger() << "Сервер прослушивание на порту: " + std::to_string(_port);

}


int NetworkServer::acceptClient()  {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    // Сохраняем клиентский сокет в _client_socket
    int _client_socket = accept(_server_fd, (sockaddr*)&client_addr, &addr_len);

    if (_client_socket < 0){
        return 0;
    }
    
    get_logger() << "Клиент подключен к сокету: " + std::to_string(_client_socket);
    return _client_socket;
}


void NetworkServer::sendMess(const std::string& mess)  {
    // std::string senMess = mess ; // Добавляем делимитер (RecordShort Separator + newline) + "\x1E\n"
    int result = send(currentUser.client_socket, mess.c_str(), mess.size(), 0);
    // Логирование сообщений пользователей JSON (для отладки) в отдельный файл
    std::string log_mess_socket = "Отправка сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + mess;
    get_logger().logMessageUser(log_mess_socket);
    std::cout << get_logger().getLastLineLogUsers() << std::endl;
    
    if (result <= 0) {
        throw std::runtime_error("Ошибка отправки Mess");
    }
}


// Устаревший метод: читает всё, что есть в сокете (до закрытия соединения клиентом)
// std::string NetworkServer::getMess() {
//     char buffer[1991024] = {0};

//     int bytes_read = recv(currentUser.client_socket, buffer, sizeof(buffer), 0);

//     if (bytes_read <= 0) 
//         throw  std::runtime_error("Ошибка в получении сообщения или закрыто соединение клиентом!");

//     // Логирование сообщений пользователей JSON (для отладки) в отдельный файл
//     //Для логирования только сообщений (не JSON) можно было добавить строку  get_logger().logMessageUser(сообщение); 
//     //в методы handle классов HandlerMessage№, но логирование JSON в данном случае мне показалось полезнее для выявления ошибок.
//     std::string log_mess_socket = "Принято сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + std::string(buffer, bytes_read);
//     get_logger().logMessageUser(log_mess_socket);
//     std::cout << get_logger().getLastLineLogUsers() << std::endl;
    
//     return std::string(buffer, bytes_read);
// }

// Метод: возвращает первый полный JSON из потока (оставляет остаток в currentUser.recv_buffer_)
// Бросает std::runtime_error при закрытии соединения или фатальной ошибке.
std::string NetworkServer::getMess() {
    const size_t TMP_BUF = 64 * 1024;
    char tmp[TMP_BUF];

    // Вспомогательная функция: найти диапазон (start,end inclusive) первого полного JSON в buf.
    // Возвращает {npos,npos}, если полного JSON нет.
    auto extract_one_json_range = [&](const std::string &buf) -> std::pair<size_t, size_t> {
        size_t n = buf.size();
        size_t i = 0;
        // пропускаем пробелы
        while (i < n && isspace(static_cast<unsigned char>(buf[i]))) ++i;
        // ищем начало JSON: '{' или '['
        while (i < n && buf[i] != '{' && buf[i] != '[') ++i;
        if (i >= n) return {std::string::npos, std::string::npos}; // нет JSON

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
                    if (depth == 0) return {i, p}; // нашли полный JSON
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
        return {std::string::npos, std::string::npos}; // нет полного JSON
    };

    // 1) если в буфере уже есть полный JSON — вернуть его сразу
    auto r = extract_one_json_range(currentUser.recv_buffer_);
    if (r.first != std::string::npos) {
        size_t start = r.first, end = r.second;
        std::string msg = currentUser.recv_buffer_.substr(start, end - start + 1);
        currentUser.recv_buffer_.erase(0, end + 1);

        // Логирование принятого JSON
        std::string log_mess_socket = "Принято сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + msg;
        get_logger().logMessageUser(log_mess_socket);
        std::cout << get_logger().getLastLineLogUsers() << std::endl;

        return msg;
    }

    // 2) читаем из сокета, накапливаем и пробуем снова
    while (true) {
        ssize_t bytes_read = ::recv(currentUser.client_socket, tmp, sizeof(tmp), 0); // блокирующий вызов
        if (bytes_read > 0) { 
            currentUser.recv_buffer_.append(tmp, static_cast<size_t>(bytes_read));

            // защита от чрезмерно больших сообщений
            const size_t MAX_ALLOWED = 32ULL * 1024ULL * 1024ULL; // 32 MB
            if (currentUser.recv_buffer_.size() > MAX_ALLOWED) {
                get_logger() << "Получено слишком большое сообщение: " + std::to_string(currentUser.recv_buffer_.size()) + " байт";
                throw std::runtime_error("Сообщение превышает допустимый размер");
            }

            // попытка извлечь JSON
            r = extract_one_json_range(currentUser.recv_buffer_);
            if (r.first != std::string::npos) {
                size_t start = r.first, end = r.second;
                std::string msg = currentUser.recv_buffer_.substr(start, end - start + 1);
                currentUser.recv_buffer_.erase(0, end + 1);

                std::string log_mess_socket = "Принято сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + msg;
                get_logger().logMessageUser(log_mess_socket);
                std::cout << get_logger().getLastLineLogUsers() << std::endl;

                return msg;
            }

            // иначе — продолжаем читать (в блок сценарии recv будет ждать следующих байт)
            continue;
        } else if (bytes_read == 0) {
            // клиент закрыл соединение
            get_logger() << "Клиент закрыл соединение на сокете: " + std::to_string(currentUser.client_socket);
            throw std::runtime_error("Клиент закрыл соединение");
        } else { // bytes_read < 0 ошибка
            if (errno == EINTR) {
                continue; // повторяем recv
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) { // Если сокет странет НЕБЛОКИРУЮЩЕМ 
                // если сокет НЕБЛОКИРУЮЩИЙ — дождёмся доступности с помощью select
                fd_set rf;
                FD_ZERO(&rf);
                FD_SET(currentUser.client_socket, &rf);
                timeval tv{5, 0}; // ждём до 5 секунд
                int sel = select(currentUser.client_socket + 1, &rf, nullptr, nullptr, &tv);
                if (sel <= 0) {
                    // timeout или ошибка — вернёмся к чтению (попробуем снова); в синхронной модели это редко нужно
                    continue;
                } else {
                    // данные теперь доступны — следующая итерация recv вернёт их
                    continue;
                }
            }
            // другая ошибка
            std::string err = "recv error: ";
            err += strerror(errno);
            get_logger() << err;
            throw std::runtime_error(err);
        }
    } // while
}