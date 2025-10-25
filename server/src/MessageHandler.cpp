#include "MessageHandler.h"
#include "NetworkServer.h"
#include "BD.h"
#include "Message.h"
#include <nlohmann/json.hpp>
#include "sha1.h"
#include <string>
#include <variant>
#include <memory>
#include "Core.h"
#include <utility>
#include "dataUserOnline.h"
#include "Logger.h"


bool MessageHandler::handleNext(const std::shared_ptr<Message>& message) {
    if (_next) return _next->handle(message);
    return false;
}


// Обработка для Message1 (авторицация)
bool HandlerMessage1::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 1) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    auto m1 = std::dynamic_pointer_cast<Message1>(message);
    
    // Логика обработки
    std::shared_ptr<User> user = currentUser.db->search_User(m1->login);

    // Проверка на существование пользователя и правильность пароля
    bool authSuccess = user && (hashToString(sha1(m1->pass)) == user->getPass());
    
    // Если аутентификация успешна, проверяем бан
    if (authSuccess) {
        bool isBanned = false;
        if (currentUser.db->checkBanByLogin(m1->login, isBanned)) {
            // Ошибка проверки бана
            Message50 response;
            response.status_request = false;
            json j;
            response.to_json(j);
            _network->sendMess(j.dump());
            throw std::runtime_error("HandlerMessage1: Ошибка проверки бана");
        }
        if (isBanned) {
            // Пользователь забанен
            Message50 response;
            response.status_request = false;
            json j;
            response.to_json(j);
            _network->sendMess(j.dump());
            throw std::runtime_error("HandlerMessage1: Закрываю соединение... БАН");
        }
        
        // Если не забанен, снимаем флаг discon
        if (currentUser.db->setDisconByLogin(m1->login, false)) {
            // Ошибка снятия discon
            Message50 response;
            response.status_request = false;
            json j;
            response.to_json(j);
            _network->sendMess(j.dump());
            throw std::runtime_error("HandlerMessage1: Ошибка снятия флага discon");
        }
    }
    
    // Формируем ответ
    Message56 response;
    response.authorization = authSuccess;
    //Фиксация авторизации
    if (authSuccess){
        currentUser.online_user_login = user->getLogin();
        response.my_login = currentUser.online_user_login;
        response.my_name = user->getName();    
    }
    
    // Отправляем ответ через сеть
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    
    return true;  // Сообщение обработано
}

// Обработчик для Message2 (регистрация)
bool HandlerMessage2::handle(const std::shared_ptr<Message>& message) {
    if (message->getTupe() != 2) {
        return handleNext(message);
    }
    
    auto m2 = std::dynamic_pointer_cast<Message2>(message);
    
    if (m2->name.empty() || m2->login.empty() || m2->pass.empty()) {
        Message56 response;
        response.authorization = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        return true;
    }
    
    if (currentUser.db->search_User(m2->login)) {
        Message55 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        return true;
    }

    
    std::shared_ptr<User> user = std::make_shared<User>(m2->login, hashToString(sha1(m2->pass)), m2->name);
    currentUser.db->write_User(user);
    //Фиксация авторизации
    currentUser.online_user_login = user->getLogin();

    Message56 response;
    response.authorization = true;
    response.my_login = currentUser.online_user_login;
    response.my_name = user->getName();
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    
    return true;
}



// Обработчик для Message3 (Передача данных приватного чата)
bool HandlerMessage3::handle(const std::shared_ptr<Message>& message){
    if (message->getTupe() != 3) {
        return handleNext(message);
    }
    
    auto m3 = std::dynamic_pointer_cast<Message3>(message);
    std::shared_ptr<User> user_sender = currentUser.db->search_User(m3->user_sender);
    std::shared_ptr<User> user_recipient = currentUser.db->search_User(m3->user_recipient);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение... Разлогирован");
    }
    
    if (user_sender == nullptr || user_recipient == nullptr)
    {
        get_logger() << "Ошибка БД - HandlerMessage3::handle";
        //Error: Не верные данные авторизации авторизованного юзера (сообщение 3)
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение...");
    }

    if(currentUser.online_user_login !=  user_sender->getLogin()){
        get_logger() << "Пользователь присылает не верные данные или он не авторизован";
        //Error: Попытка получить ответ не авторизованного юзера (сообщение 3)"
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение...");
    }

    currentUser.db->write_Chat_P(user_sender, user_recipient, m3->mess);
    std::vector<MessageStruct> history_chat_P;
    currentUser.db->load_Chat_P(user_sender, user_recipient, history_chat_P);

    // Отправляем ответ
    Message52 mess_class;
    mess_class.history_chat_P = history_chat_P;
    mess_class.login_name_MY.first = user_sender->getLogin();
    mess_class.login_name_MY.second = user_sender->getName();
    mess_class.login_name_friend.first = user_recipient->getLogin();
    mess_class.login_name_friend.second = user_recipient->getName();
    json mess_json;
    json j;
    mess_class.to_json(j);
    _network->sendMess(j.dump());
    
    return true;
}


// Обработчик для Message4 (Передача данных общего чата)
bool HandlerMessage4::handle(const std::shared_ptr<Message>& message){
    
    if (message->getTupe() != 4) {
        return handleNext(message);
    }

    auto m4 = std::dynamic_pointer_cast<Message4>(message);
    std::shared_ptr<User> user_sender = currentUser.db->search_User(m4->login_user_sender);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение... Разлогирован");
    }
    
    if (user_sender == nullptr)
    {
        get_logger() << "Error: Не верные данные авторизации авторизованного юзера (сообщение 4)";
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение...");
    }

    if(currentUser.online_user_login !=  user_sender->getLogin()){
        get_logger() << "Пользователь присылает не верные данные или он не авторизован";
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение...");
    }

    currentUser.db->write_Chat_H(user_sender, m4->mess);
    std::vector<MessageStruct> history_chat_H; 
    currentUser.db->load_Chat_H(history_chat_H);
    
    // Отправляем ответ
    Message51 mess_class;
    mess_class.history_chat_H = history_chat_H;
    json mess_json;
    mess_class.to_json(mess_json);
    _network->sendMess(mess_json.dump());
    return true;
}


// Обработчик для Message5 (Запрос на получение списка приватных чатов)
// Если в БД нет данных, то передаем пустоту, пользователь сам должен проверить, что сообщений нет
bool HandlerMessage5::handle(const std::shared_ptr<Message>& message){
    
    if (message->getTupe() != 5) {
        return handleNext(message);
    }

    auto m5 = std::dynamic_pointer_cast<Message5>(message);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage5: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage5: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage5: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage5: Закрываю соединение... Разлогирован");
    }
    
    //получаем пользователя и проверяем на nullptr
    std::shared_ptr<User> user_sender = currentUser.db->search_User(m5->my_login);

    //есть ли пользователь в базе и логин залогированного пользователя?
    if (user_sender && currentUser.online_user_login == user_sender->getLogin())
    {
        // получаем данные с БД
        std::vector<std::pair<std::string, std::string>> list_Chat_P = currentUser.db->my_chat_P(m5->my_login);
        
        // Отправляем ответ
        Message53 mess_class;
        mess_class.list_chat_P = list_Chat_P;
        json mess_json;
        mess_class.to_json(mess_json);
        _network->sendMess(mess_json.dump());
        return true;
    }
    
    // Отправляем ответ об ошибке
    get_logger() << "Пользователь присылает не верные данные или он не авторизован";
    // Отправляем ответ об ошибке
    Message50 response;
    response.status_request = false;
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    throw std::runtime_error("HandlerMessage5: Закрываю соединение...");
}


// Обработчик для Message6 (Запрос на получение списока юзеров кому написать)
// Если в БД нет данных, то передаем пустоту, пользователь сам должен проверить, что сообщений нет
bool HandlerMessage6::handle(const std::shared_ptr<Message>& message){
    
    if (message->getTupe() != 6) {
        return handleNext(message);
    }

    auto m6 = std::dynamic_pointer_cast<Message6>(message);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage6: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage6: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage6: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage6: Закрываю соединение... Разлогирован");
    }

    std::shared_ptr<User> user_sender = currentUser.db->search_User(m6->my_login);
    
    //есть ли пользователь в базе и логин залогированного пользователя?
    if (user_sender && currentUser.online_user_login == user_sender->getLogin())
    {
        // получаем данные с БД
        std::vector<std::pair<std::string, std::string>> list_Users = currentUser.db->list_all_User(m6->my_login);
        
        // Отправляем ответ
        Message54 mess_class;
        mess_class.list_Users = list_Users;
        json mess_json;
        mess_class.to_json(mess_json);
        _network->sendMess(mess_json.dump());
        return true;
    }
    
    // Отправляем ответ об ошибке
    get_logger() << "Пользователь присылает не верные данные или он не авторизован";
    // Отправляем ответ об ошибке
    Message50 response;
    response.status_request = false;
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    throw std::runtime_error("HandlerMessage6: Закрываю соединение...");
}


// Запрос юзера получить свое имя
bool HandlerMessage7::handle(const std::shared_ptr<Message>& message){
    
    if (message->getTupe() != 7) {
        return handleNext(message);
    }

    auto m7 = std::dynamic_pointer_cast<Message7>(message);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage7: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage7: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage7: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage7: Закрываю соединение... Разлогирован");
    }
    
    //получаем пользователя и проверяем на nullptr
    std::shared_ptr<User> user = currentUser.db->search_User(m7->my_login);

    //есть ли пользователь в базе и логин залогированного пользователя?
    if (user && currentUser.online_user_login == user->getLogin())
    {
        // Отправляем ответ
        Message56 mess_class;
        mess_class.my_name = user->getName();
        json mess_json;
        mess_class.to_json(mess_json);
        _network->sendMess(mess_json.dump());
        return true;
    }
    
    // Отправляем ответ об ошибке
    get_logger() << "Ошибка, позможные причины:\n- Пользователь присылает не верные данные\n- Не авторизован\n- Нет данных в БД";
    // Отправляем ответ об ошибке
    Message50 response;
    response.status_request = false;
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    throw std::runtime_error("HandlerMessage7: Закрываю соединение...");
}


// Обработчик для Message8 (обновить данные приватного чата)
bool HandlerMessage8::handle(const std::shared_ptr<Message>& message){
    if (message->getTupe() != 8) {
        return handleNext(message);
    }
    
    auto m8 = std::dynamic_pointer_cast<Message8>(message);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage8: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage8: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage8: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage8: Закрываю соединение... Разлогирован");
    }
    
    std::shared_ptr<User> user_sender = currentUser.db->search_User(m8->user_sender);
    std::shared_ptr<User> user_recipient = currentUser.db->search_User(m8->user_recipient);
    
    if (user_sender == nullptr || user_recipient == nullptr)
    {
        get_logger() << "ошибка бд: HandlerMessage8::handle";
        //Error: Не верные данные авторизации авторизованного юзера (сообщение 3)
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение...");
    }

    if(currentUser.online_user_login !=  user_sender->getLogin()){
        get_logger() << "Пользователь присылает не верные данные или он не авторизован";
        //Error: Попытка получить ответ не авторизованного юзера (сообщение 3)"
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage3: Закрываю соединение...");
    }

    std::vector<MessageStruct> history_chat_P; 
    currentUser.db->load_Chat_P(user_sender, user_recipient, history_chat_P);

    // Отправляем ответ
    Message52 mess_class;
    mess_class.history_chat_P = history_chat_P;
    mess_class.login_name_MY.first = user_sender->getLogin();
    mess_class.login_name_MY.second = user_sender->getName();
    mess_class.login_name_friend.first = user_recipient->getLogin();
    mess_class.login_name_friend.second = user_recipient->getName();
    json mess_json;
    json j;
    mess_class.to_json(j);
    _network->sendMess(j.dump());
    
    return true;
}


// Обработчик для Message9 (обновить данные общего чата)
bool HandlerMessage9::handle(const std::shared_ptr<Message>& message){
    
    if (message->getTupe() != 9) {
        return handleNext(message);
    }

    auto m9 = std::dynamic_pointer_cast<Message9>(message);
    
    // Проверка на бан и discon
    bool isBanned = false;
    bool isDiscon = false;
    
    // Проверяем бан
    if (currentUser.db->checkBanByLogin(currentUser.online_user_login, isBanned)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage9: Ошибка проверки бана");
    }
    if (isBanned) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage9: Закрываю соединение... БАН");
    }
    
    // Проверяем discon
    if (currentUser.db->checkDisconByLogin(currentUser.online_user_login, isDiscon)) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage9: Ошибка проверки discon");
    }
    if (isDiscon) {
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage9: Закрываю соединение... Разлогирован");
    }
    
    std::shared_ptr<User> user_sender = currentUser.db->search_User(m9->user_sender);
    
    if (user_sender == nullptr)
    {
        get_logger() << "Error: Не верные данные авторизации авторизованного юзера (сообщение 4)";
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение...");
    }

    if(currentUser.online_user_login !=  user_sender->getLogin()){
        get_logger() << "Пользователь присылает не верные данные или он не авторизован";
        // Отправляем ответ об ошибке
        Message50 response;
        response.status_request = false;
        json j;
        response.to_json(j);
        _network->sendMess(j.dump());
        throw std::runtime_error("HandlerMessage4: Закрываю соединение...");
    }

    std::vector<MessageStruct> history_chat_H; 
    currentUser.db->load_Chat_H(history_chat_H);
    
    // Отправляем ответ
    Message51 mess_class;
    mess_class.history_chat_H = history_chat_H;
    json mess_json;
    mess_class.to_json(mess_json);
    _network->sendMess(mess_json.dump());
    return true;
}


// Обработчик для неизвестных сообщений
bool HandlerErr::handle(const std::shared_ptr<Message>& message) {
    get_logger() << "Неизвестный тип сообщения: " << message->getTupe();
    
    Message50 response;
    response.status_request = false;
    
    json j;
    response.to_json(j);
    _network->sendMess(j.dump());
    
    return true;
}
