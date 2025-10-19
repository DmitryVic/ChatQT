#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Подключаем библиотеку Winsock
    typedef SOCKET socket_t;         // Определяем тип сокета для Windows
#else
    #include <unistd.h>             //базовые функции для работы с системой Linux
    #include <sys/socket.h>         //для работы с сокетами
    #include <netinet/in.h>         //содержит структуры и константы для работы с протоколами
    #include <stdexcept>            //исключения
    #include <arpa/inet.h>          // преобразовать  ip inet_pton()
    typedef int socket_t;           // Определяем тип сокета для Linux
#endif


class Mediator; // forward declaration

class NetworkClient {
private:
    socket_t sock;              // Единственный дескриптор сокета для общения с сервером
    std::string server_ip;      // IP-адрес сервера для подключения
    int port;                   // Порт сервера для подключения

    std::shared_ptr<Mediator> mediator; // Умный указатель на Mediator
    std::thread recvThread;     // Поток для приёма сообщений
    std::thread sendThread;     // Поток для отправки сообщений
    std::atomic<bool> threadsRunning{false};


    std::string recv_buffer_; // Буфер для накопления данных при приёме

public:
    // Конструктор: принимает shared_ptr<Mediator>
    NetworkClient(const std::string& ip, int port, std::shared_ptr<Mediator> mediator);
    ~NetworkClient();

    // Метод подключения к серверу
    void connecting();

    // Запуск потоков обмена
    void startThreads();
    // Остановка потоков
    void stopThreads();

    // ОТПРАВКА СООБЩЕНИЯ (синхронно)
    void sendMess(const std::string& message);
    // ПОЛУЧЕНИЕ СООБЩЕНИЯ (синхронно)
    std::string getMess();

private:
    // Функция для потока приёма
    void recvLoop();
    // Функция для потока отправки
    void sendLoop();
};