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
    _status->setMessList(m51->history_chat_H);
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
    _status->setMessList(m52->history_chat_P);
    std::string chatName = "Открыт чат с пользователем: " + m52->login_name_friend.second;
    _status->setChatName(chatName);
    FriendData friendD;
    friendD.login = m52->login_name_friend.first;
    friendD.name = m52->login_name_friend.second;
    _status->setFriendOpenChatP(friendD);
    
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
    _status->setMess(message);
    _status->setMessType(53);
    _status->set_message_status(true);
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
    _status->setMess(message);
    _status->setMessType(54);
    _status->set_message_status(true);
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
    _status->setMenuAuthoriz(MENU_AUTHORIZATION::VOID_REG);
    _status->setMenuChat(MENU_CHAT::MENU_VOID);
    _status->setLogin("");
    _status->setPass("");
    _status->setName("");
    _II->display_message("Данный логин занят!");
    return true;
}


// Обработка для Message56 (Ответ сервера вернуть имя)
bool HandlerMessage56::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 56) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m56 = std::dynamic_pointer_cast<Message56>(message);
    this->_status->setName(m56->my_name);
    std::string hi = m56->my_name;
    hi += ", здраствуйте!";
    _II->display_message(hi);
    return true;
}

// Обработка для Message56 (Ответ сервера вернуть имя)
bool HandlerErr::handle(const std::shared_ptr<Message>& message) {
    //обрабатываем
    _status->setMenuAuthoriz(MENU_AUTHORIZATION::VOID_REG);
    _status->setMenuChat(MENU_CHAT::MENU_VOID);
    _status->setLogin("");
    _status->setPass("");
    _status->setName("");
    _II->display_message("Ошибка ответа сервера");
    return true;
}