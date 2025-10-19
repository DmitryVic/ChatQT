#include "NetworkServer.h"
#include <string>
#include <iostream>
#include <unistd.h>             //базовые функции для работы с системой Linux
#include <sys/socket.h>         //для работы с сокетами
#include <netinet/in.h>         //содержит структуры и константы для работы с протоколами
#include <stdexcept>            //исключения
#include "dataUserOnline.h"
#include "Logger.h"


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


//метод читает все данные (в отличие от прим в модуле, где обрезается до '\n')
std::string NetworkServer::getMess() {
    char buffer[1024] = {0};

    int bytes_read = recv(currentUser.client_socket, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0) 
        throw  std::runtime_error("Ошибка в получении сообщения или закрыто соединение клиентом!");

    // Логирование сообщений пользователей JSON (для отладки) в отдельный файл
    //Для логирования только сообщений (не JSON) можно было добавить строку  get_logger().logMessageUser(сообщение); 
    //в методы handle классов HandlerMessage№, но логирование JSON в данном случае мне показалось полезнее для выявления ошибок.
    std::string log_mess_socket = "Принято сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + std::string(buffer, bytes_read);
    get_logger().logMessageUser(log_mess_socket);
    std::cout << get_logger().getLastLineLogUsers() << std::endl;
    
    return std::string(buffer, bytes_read);
}


void NetworkServer::sendMess(const std::string& mess)  {
    int result = send(currentUser.client_socket, mess.c_str(), mess.size(), 0);

    // Логирование сообщений пользователей JSON (для отладки) в отдельный файл
    //Для логирования только сообщений (не JSON) можно было добавить строку  get_logger().logMessageUser(сообщение); 
    //в методы handle классов HandlerMessage№, но логирование JSON в данном случае мне показалось полезнее для выявления ошибок.
    std::string log_mess_socket = "Отправка сообщение от сокета " + std::to_string(currentUser.client_socket) + ": \t" + mess;
    get_logger().logMessageUser(log_mess_socket);
    std::cout << get_logger().getLastLineLogUsers() << std::endl;

    if (result <= 0) {
        throw std::runtime_error("Ошибка отправки Mess");
    }
}
