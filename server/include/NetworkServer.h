#pragma once
#include <string>
#include <iostream>
#include <unistd.h>             //базовые функции для работы с системой Linux
#include <sys/socket.h>         //для работы с сокетами
#include <netinet/in.h>         //содержит структуры и константы для работы с протоколами
#include <stdexcept>            //исключения
#include <thread>
#include <vector>
#include <mutex>


class NetworkServer
{
private:
    int _server_fd;         // дескриптор серверного сокета
    int _port;
    
public:
    
    explicit NetworkServer(int port) : _port(port), _server_fd(-1) {};

    ~NetworkServer() {
        if (_server_fd != -1)
        {
            close(_server_fd);
        }  
    };
    
    void start();

    int acceptClient();

    //метод читает все данные (в отличие от прим в модуле, где обрезается до '\n')
    virtual std::string getMess();

    virtual void sendMess(const std::string& mess);
};