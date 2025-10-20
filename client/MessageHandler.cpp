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

bool MessageHandler::handleNext(const std::shared_ptr<Message>& message) {
    if (_next) return _next->handle(message);
    return false;
}

// Обработка для Message50 (авторизован или ошибка)
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
    
    }
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
    auto m51 = std::dynamic_pointer_cast<Message51>(message);
    _status->setMessList(std::move(m51->history_chat_H));
    _status->setChatName("Общий чат");
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
    auto m52 = std::dynamic_pointer_cast<Message52>(message);
    _status->setMessList(std::move(m52->history_chat_P));
    std::string chatName = "Открыт чат с пользователем: " + m52->login_name_friend.second;
    _status->setChatName(std::move(chatName));
    FriendData friendD;
    friendD.login = m52->login_name_friend.first;
    friendD.name = m52->login_name_friend.second;
    _status->setFriendOpenChatP(std::move(friendD));
    
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
    auto m53 = std::dynamic_pointer_cast<Message53>(message);
    _status->setListChatP(std::move(m53->list_chat_P));
    return true;
}








// Обработка для Message53 (получить список всех юзеров в чате кому написать)
bool HandlerMessage54::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 54) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m54 = std::dynamic_pointer_cast<Message54>(message);
    _status->setListChatP(std::move(m54->list_Users));
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
    _status->setLoginBusy(true);
    return true;
}


// Обработка для Message56 ( Ответ сервера Вы залогинены)
bool HandlerMessage56::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 56) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m56 = std::dynamic_pointer_cast<Message56>(message);
    _status->setAuthorizationStatus(true);
    User us(m56->my_login, "", m56->my_name);
    _status->setUser(us);
    return true;
}

// ошибка
bool HandlerErr::handle(const std::shared_ptr<Message>& message) {
    //обрабатываем
    std::cerr << "HandlerErr::handle: не известный тип сообщения \n";
    return true;
}