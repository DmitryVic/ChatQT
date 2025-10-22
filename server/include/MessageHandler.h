#pragma once
#include "NetworkServer.h"
#include "BD.h"
#include "Message.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include "Core.h"
#include <utility>
#include "dataUserOnline.h"


// Базовый класс обработчика - метод цепочки ответственности
class MessageHandler {
protected:
    std::shared_ptr<NetworkServer> _network;
    //Паттерн Цепочка ответственности каждый будет пытаться обработать, но сможет только 1
    std::unique_ptr<MessageHandler> _next;
   

    
public:
    MessageHandler(std::shared_ptr<NetworkServer> network)
        : _network(network), _next(nullptr) {}
    
    virtual ~MessageHandler() = default;
    
    void setNext(std::unique_ptr<MessageHandler> next) {
        _next = std::move(next);
    }
    
    virtual bool handle(const std::shared_ptr<Message>& message) = 0;
    
    bool handleNext(const std::shared_ptr<Message>& message);
};

// Обработка для Message1 (авторицация)
class HandlerMessage1 : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message2 (регистрация)
class HandlerMessage2 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message3 (Передача данных приватного чата)
class HandlerMessage3 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message4 (Передача данных общего чата)
class HandlerMessage4 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message5 (Запрос на получение списка приватных чатов)
class HandlerMessage5 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message6 (Запрос на получение списока юзеров кому написать)
class HandlerMessage6 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message7 (Запрос юзера получить свое имя)
class HandlerMessage7 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};

// Обработчик для Message8 (обновить данные приватного чата)
class HandlerMessage8 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для Message9 (обновить данные общего чата)
class HandlerMessage9 : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};


// Обработчик для неизвестных сообщений
class HandlerErr : public MessageHandler {
public:
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};