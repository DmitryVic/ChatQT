#include "MessageHandler.h"
#include "NetworkClient.h"
#include "Message.h"
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <utility>
#include <memory>
#include "UserStatus.h"
#include "MessageHandler.h"
#include "Logger.h"

bool MessageHandler::handleNext(const std::shared_ptr<Message>& message) {
    if (_next) return _next->handle(message);
    return false;
}

// Обработка для Message50 (ошибка от сервера)
bool HandlerMessage50::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 50) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m50 = std::dynamic_pointer_cast<Message50>(message);
    
    if (m50->status_request)
    {
        _status->stopApp();
    }
    else
    {
        // Пока ничего, это вообще теперь только ошибка авторизация на 56
        _status->stopApp();
    }
    get_logger() << "HandlerMessage50 | Ошибка 50";
    return true;
}


// Обработка для Message51 (Передача данных общего чата)
bool HandlerMessage51::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 51) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    get_logger() << "HandlerMessage51 | Пришло сообщение Message51 | Игнорирую";
    return true;
}

// Обработка для Message52 (Передача данных приватного чата)
bool HandlerMessage52::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 52) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    get_logger() << "HandlerMessage52 | Пришло сообщение Message52 | Игнорирую";
    return true;
}



// Обработка для Message53 (Передача списка истории приватных чатов)
bool HandlerMessage53::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 53) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    get_logger() << "HandlerMessage53 | Пришло сообщение Message53 | Игнорирую";
    return true;
}


// Обработка для Message54 (получить список всех юзеров в чате кому написать)
bool HandlerMessage54::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 54) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    get_logger() << "HandlerMessage54 | Пришло сообщение Message54 | Игнорирую";
    return true;
}


// Обработка для Message53 (Ответ сервера логин занят)
bool HandlerMessage55::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 55) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    get_logger() << "HandlerMessage55 | Пришло сообщение Message53 | Игнорирую";
    return true;
}


// Обработка для Message56 (Вы залогинены)
bool HandlerMessage56::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 56) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m56 = std::dynamic_pointer_cast<Message56>(message);
    _status->setServerResponseReg(true); // Ответ пришел
    if (m56->authorization)
    {
        _status->setAuthorizationStatus(true);
        get_logger() << "HandlerMessage56 | Выполнен вход";
    }
    else
    {
        _status->setAuthorizationStatus(false);
        get_logger() << "HandlerMessage56 | Ошибка входа, проверь сервер\n  m56->authorization - false";

    } 
    return true;
}


// Обработка для Message57 ответ на discon user
bool HandlerMessage57::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 57) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m57 = std::dynamic_pointer_cast<Message57>(message);

    return true;
}


// Обработка для Message58  ответ на ban user
bool HandlerMessage56::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 58) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m58 = std::dynamic_pointer_cast<Message58>(message);

    return true;
}

// Обработка для Message59 ответ на запрос списка юзеров ADMIN
bool HandlerMessage59::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 59) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m59 = std::dynamic_pointer_cast<Message59>(message);

    return true;
}


//Обработка для Message60 ADMIN ответ на запрос списка сообщений
bool HandlerMessage60::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 60) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m60 = std::dynamic_pointer_cast<Message60>(message);

    return true;
}


// ошибка
bool HandlerErr::handle(const std::shared_ptr<Message>& message) {
    //обрабатываем
    get_logger() << "HandlerErr::handle: не известный тип сообщения \n" << message;
    return true;
}
