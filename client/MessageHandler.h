#pragma once
#include "Message.h"
#include "NetworkClient.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include "Mediator.h"



class MessageHandler {
protected:
    std::shared_ptr<MessageHandler> _next;
    std::shared_ptr<Mediator> _Mediator;
   
public:
    MessageHandler(
    std::shared_ptr<Mediator> Mediator
) : _Mediator(Mediator), _next(nullptr) {};
    
    virtual ~MessageHandler() = default;
    
    void setNext(std::shared_ptr<MessageHandler> next) {
        _next = next;
    }
    
    virtual bool handle(const std::shared_ptr<Message>& message) = 0;
    
    bool handleNext(const std::shared_ptr<Message>& message);
    
};


// Обработка для Message101 
// Сообщение о наличии (true) или отсутствие (false) критических ошибок сервера
class HandlerMessage101 : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;

};

// Обработка для Message102 
// Получена запись
class HandlerMessage102 : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;

};

// Обработка для Message103
// Запись добавлена
class HandlerMessage103 : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;

};

// Обработка для Message104
// Запись добавлена
class HandlerMessage104 : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;

};


 
class HandlerErr : public MessageHandler {
public:
    //using для наследования конструкторов базового класса
    using MessageHandler::MessageHandler;
    
    bool handle(const std::shared_ptr<Message>& message) override;
};