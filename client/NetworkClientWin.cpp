// Реализация NetworkClient для Windows
#include <string>
#include <iostream>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "NetworkClient.h"
#include "UserStatus.h"


NetworkClient::NetworkClient(const std::string& ip, int port, std::shared_ptr<UserStatus> status)
    : server_ip(ip), port(port), sock(INVALID_SOCKET), status(std::move(status)) {}

NetworkClient::~NetworkClient() {
	stopThreads();
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
		std::cerr << "Соединение с сервером закрыто" << std::endl;
	}
	WSACleanup();
}

// Запуск потоков приёма и отправки
void NetworkClient::startThreads() {
	if (threadsRunning.load()) return;
	threadsRunning.store(true);
	recvThread = std::thread(&NetworkClient::recvLoop, this);
	sendThread = std::thread(&NetworkClient::sendLoop, this);
        status->setNetworckThreadsSost(true);
    std::cerr << "Потоки запущены!!!!!!!!" << std::endl;
}

// Остановка потоков приёма и отправки
void NetworkClient::stopThreads() {
	if (!threadsRunning.load()) return;
	threadsRunning.store(false);
	if (recvThread.joinable()) recvThread.join();
	if (sendThread.joinable()) sendThread.join();
        status->setNetworckThreadsSost(false);
    std::cerr << "Потоки остановлены!!!!!!!!" << std::endl;
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
		int sel = select(0, &readfds, nullptr, nullptr, &timeout);
		// sel > 0 - сокет готов к чтению
		// sel == 0 - таймаут
		if (sel > 0 && FD_ISSET(sock, &readfds)) {
			try {
				std::string msg = getMess();


				if (msg == "") {
                    // Закрытие соединения
                    status->setNetworckConnect(false);
					break;
                }
                status->setNetworckConnect(true); // соединение реально установлено

                // если соединение оборвано,, то getMess будет постоянно отдавать ""
                // НУЖНО ДОБАВИТЬ ОСТАНОВКУ И ПЕРЕПРОВЕРКУ В ЦИКЛЕ 

				if (!msg.empty()) {
                                        status->pushAcceptedMessage(msg);
				}
			} catch (const std::exception& e) {
				std::cerr << "Ошибка приёма: " << e.what() << std::endl;
				break;
			}
		} else if (sel == 0) {
			// timeout, сообщение не пришло
			// пропускаем для  того чтобы перепроверить флаг работоспособности
			continue;
		} else if (sel < 0) {
			std::cerr << "Ошибка select() в recvLoop" << std::endl;
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
				std::cerr << "Ошибка отправки: " << e.what() << std::endl;
				break;
			}
		}
	}
}

// Метод подключения к серверу Win дополнительно выполняем select что бы понять что TCP действительно подключенн
void NetworkClient::connecting() {
    bool err = false;
    
    // Добавляем проверку на уже существующий сокет
    if (sock != INVALID_SOCKET) {
        closesocket(sock);  // Закрываем старый сокет
        sock = INVALID_SOCKET;
    }

    // Инициализация Winsock должна происходить только один раз
    static bool wsaInitialized = false;
    if (!wsaInitialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            status->setNetworckConnect(false);
            err = true;
            std::cerr << "Ошибка инициализации WinSock\n";
            throw std::runtime_error("Ошибка инициализации WinSock");
        }
        wsaInitialized = true;
    }

    // Создание нового сокета
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        status->setNetworckConnect(false);
        err = true;
        std::cerr << "Не удалось создать сокет\n";
        throw std::runtime_error("Не удалось создать сокет");
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Проверка IP-адреса
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        closesocket(sock);
        std::cerr << "Некорректный адрес сервера\n";
        status->setNetworckConnect(false);
        err = true;
    }

    // Подключение к серверу
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        if (errorCode != WSAEINPROGRESS) {  // Игнорируем асинхронные ошибки
            closesocket(sock);
            std::cerr << "Ошибка подключения: " << errorCode << "\n";
            status->setNetworckConnect(false);
            err = true;
        }
    }

    if (!err) {
        // Добавляем проверку состояния подключения
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500 мс

        int sel = select(0, &readfds, nullptr, nullptr, &timeout);

        if (sel < 0) {
            int selErr = WSAGetLastError();
            std::cerr << "select() error: " << selErr << "\n";
            closesocket(sock);
            status->setNetworckConnect(false);
        }
        else if (sel == 0) {
            // таймаут — соединение считается установленным
            status->setNetworckConnect(true);
        }
        else { // sel > 0
            if (FD_ISSET(sock, &readfds)) {
                // читаем 1 байт без удаления из буфера (MSG_PEEK)
                char peekBuf;
                int r = recv(sock, &peekBuf, 1, MSG_PEEK);
                if (r == 0) {
                    // сервер закрыл соединение
                    std::cerr << "Peer закрыл соединение сразу после connect()\n";
                    closesocket(sock);
                    status->setNetworckConnect(false);
                } else if (r > 0) {
                    // пришли данные — считаем handshake успешным
                    status->setNetworckConnect(true);
                    std::cerr << "CONNECT ON\n";
                } else { // r == SOCKET_ERROR
                    int recvErr = WSAGetLastError();
                    if (recvErr == WSAEWOULDBLOCK) {
                        // гонка — treat as success
                        status->setNetworckConnect(true);
                    } else {
                        std::cerr << "recv(MSG_PEEK) error: " << recvErr << "\n";
                        closesocket(sock);
                        status->setNetworckConnect(false);
                    }
                }
            } else {
                // маловероятная ситуация
                closesocket(sock);
                status->setNetworckConnect(false);
            }
        }
    }
}



// ОТПРАВКА СООБЩЕНИЯ
void NetworkClient::sendMess(const std::string& message) {
	if (send(sock, message.c_str(), message.size(), 0) == SOCKET_ERROR){
		std::cerr << "Ошибка отправки\n";
                status->setNetworckConnect(false);
		// throw std::runtime_error("Ошибка отправки");
	}
}


// ПОЛУЧЕНИЕ СООБЩЕНИЯ

// СТАРЫЙ ВАРИАНТ getMess
/*
std::string NetworkClient::getMess() {
    char buffer[1024] = {0};
    int bytes_read = recv(sock, buffer, sizeof(buffer), 0);

    if (bytes_read == SOCKET_ERROR) throw std::runtime_error("Ошибка чтения");
    if (bytes_read == 0) {
        std::cerr << "getMess | Сервер закрыл соединение\n";
        status->setNetworckConnect(false);
        // throw std::runtime_error("Сервер закрыл соединение");
        // если соединение оборвано,, то getMess будет постоянно отдавать ""
        // НУЖНО ДОБАВИТЬ ОСТАНОВКУ И ПЕРЕПРОВЕРКУ В ЦИКЛЕ 
    }

    return std::string(buffer, bytes_read);
}
*/

// Новый вариант getMess с поддержкой буфера и извлечения одного JSON-сообщения (адаптация с Linux)
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
        int n = ::recv(sock, tmp, sizeof(tmp), 0);      // читаем из сокета
        if (n > 0) {                                        // получили n байт
            recv_buffer_.append(tmp, static_cast<size_t>(n));

            const size_t MAX_ALLOWED = 32 * 1024 * 1024;    // 32 MB
            if (recv_buffer_.size() > MAX_ALLOWED) {        // защитный лимит
                std::cerr << "Получено слишком большое сообщение: " << recv_buffer_.size() << " байт\n";
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
        } else if (n == 0) { // соединение закрыто
            std::cerr << "getMess | Сервер закрыл соединение\n";
            status->setNetworckConnect(false);
            return "";
        } else { // n < 0 - ошибка
            int err = WSAGetLastError();
            if (err == WSAEINTR) {
                continue;
            } else if (err == WSAEWOULDBLOCK) {
                // если сокет неблокирующий — можно подождать или просто продолжить
                continue;
            } else {
                std::cerr << "recv error: " << err << "\n";
                status->setNetworckConnect(false);
                throw std::runtime_error("Ошибка чтения сокета");
            }
        }
    } // while

    // если флаги работы изменились — выходим
    return "";
}
