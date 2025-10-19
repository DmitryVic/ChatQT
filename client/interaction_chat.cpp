#include <string>
#include <iostream>
#include <memory>
#include <variant>
#include <utility>
#include "Message.h"
#include "UserStatus.h"
#include "NetworkClient.h"
#include "interactive_interface.h"
#include "interaction_chat.h"
#include <nlohmann/json.hpp>
#include "MessageHandler.h"


using json = nlohmann::json;

interaction_chat::interaction_chat(
    std::shared_ptr<NetworkClient> network, 
                   std::shared_ptr<interactive_interface> II, 
                   std::shared_ptr<MessageHandler> hendl,
                   std::shared_ptr<UserStatus> status)
                    : _network(network), _II(II), _hendl(hendl), _status(status) 
{
}


void interaction_chat::start(){
    while (true) {
        

        switch (_status->getMenuAuthoriz())
        {
        case MENU_AUTHORIZATION::EXIT_PROGRAMM:
            return;
        case MENU_AUTHORIZATION::AUTHORIZATION_SUCCESSFUL:{
            if (_status->getMenuChat() == MENU_CHAT::EXIT){
                _status->setMenuAuthoriz(MENU_AUTHORIZATION::EXIT_PROGRAMM);
            }
            else{
                this->menu_chat();
            }
            
            break;}
        case MENU_AUTHORIZATION::AUTHORIZATION:{
            std::shared_ptr<Message1> mes = _II->authorization(_status);
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());     
            this->getMess();
            if (_status->getMenuAuthoriz() == MENU_AUTHORIZATION::AUTHORIZATION_SUCCESSFUL){
                std::shared_ptr<Message7> mes2 = std::make_shared<Message7>();
                mes2->my_login = _status->getLogin();
                json jj;
                mes2->to_json(jj);
                _network->sendMess(jj.dump());
                this->getMess();
            }
            break;}
        case MENU_AUTHORIZATION::REG:{
            std::shared_ptr<Message2> mes = _II->reg(_status);
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());
            this->getMess();
            if (_status->getMenuAuthoriz() == MENU_AUTHORIZATION::AUTHORIZATION_SUCCESSFUL)
            {
                std::string hi = _status->getName();
                hi += ", здраствуйте!";
                _II->display_message(hi);
            }
            break;}
        case MENU_AUTHORIZATION::VOID_REG:{
            _II->show_menu_authorization(_status);
            break;}
        
        default:
            _status->setMenuAuthoriz(MENU_AUTHORIZATION::EXIT_PROGRAMM);
            return;
        }
    }
}


void interaction_chat::menu_chat() {
    while (true) {
        
        if (_status->getMenuAuthoriz() != MENU_AUTHORIZATION::AUTHORIZATION_SUCCESSFUL){
            return;
        }
    
        switch (_status->getMenuChat()) {
            case MENU_CHAT::EXIT:
                return;
            case MENU_CHAT::MENU_VOID: {
                _II->show_chat_menu(this->_status);
                break;
            }
            case MENU_CHAT::SHOW_CHAT_H: {
                this->chat_H();
                break;
            }
            case MENU_CHAT::LIST_CHAT_P: {
                this->list_chat_P();
                break;
            }
            case MENU_CHAT::LIST_USERS: {
                this->list_user();
                break;
            }
            default:
                _status->setMenuChat(MENU_CHAT::MENU_VOID);
                return;
            }
    }
}


void interaction_chat::getMess() {
    try {
        if (!_hendl) {
            throw std::runtime_error("Обработчик сообщений не инициализирован");
        }
        
        std::string json_str = _network->getMess();
        auto msg = parse_message(json_str);
        
        if (!msg) {
            throw std::runtime_error("Неверное сообщение с сервера");
        }
        
        if (!_hendl->handle(msg)) {
            throw std::runtime_error("Обработка сообщений не удалась");
        }
    } catch (const std::exception& e) {
        _II->display_message(e.what());
    }
}


void interaction_chat::chat_H(){
    while (_status->getMenuChat() == MENU_CHAT::SHOW_CHAT_H)
    {
        //Сообщение существует, откроем чат
        if(_status->get_message_status() && _status->getMessageType() == 51){
            auto m51 = std::dynamic_pointer_cast<Message51>(_status->getMessage()); // забираем сообщение
            std::shared_ptr<Message4> data = _II->show_chat_H(m51->history_chat_H, _status); //открвываем чат
            if ( data->mess == ""){//пользователь хочет выйти
                _status->set_message_status(false);
                _status->setMenuChat(MENU_CHAT::MENU_VOID);
                return;
            }
            else // пользователь не выходит обновляем чат 
            {
                std::shared_ptr<Message4> mes = std::make_shared<Message4>();
                mes->login_user_sender = this->_status->getLogin();
                mes->name_user_sender = this->_status->getName();
                mes->mess = data->mess;
                json jj;
                mes->to_json(jj);
                _network->sendMess(jj.dump());
                this->getMess();
            }
        //Не получали наше сообщение, запрашиваем еще раз данные
        }else
        {
            std::shared_ptr<Message9> mes = std::make_shared<Message9>();
            mes->user_sender = this->_status->getLogin();
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());
            this->getMess();
        }
        
    }
}


void interaction_chat::list_user(){
    while (_status->getMenuChat() == MENU_CHAT::LIST_USERS)
    {
        //Сообщение существует, откроем чат 52
        if(_status->get_message_status() && _status->getMessageType() == 54){
            auto m54 = std::dynamic_pointer_cast<Message54>(_status->getMessage()); // забираем сообщение
            _status->set_message_status(false);
            //ответ и открытие чата
            std::pair<std::string, std::string> data = _II->show_list_users(m54->list_Users, _status);

            if ( data.first == "" || data.second == ""){//пользователь хочет выйти
                _status->setMenuChat(MENU_CHAT::MENU_VOID);
                _status->set_message_status(false);
                return;
            }
            else // пользователь выбрал чат, запрашиваем данные
            {
                // запросить данные приватного чата
                std::shared_ptr<Message8> mes = std::make_shared<Message8>();
                mes->user_sender = this->_status->getLogin();
                mes->user_recipient = data.first;
                json jj;
                mes->to_json(jj);
                _network->sendMess(jj.dump());
                this->getMess();

                // открываем приватный чат
                _status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
                chat_P();
            }
        }else//Не получали сообщение список пользователей, запрашиваем
        {
            //Запрос на получение списока всех юзеров в чате кому написать
            std::shared_ptr<Message6> mes = std::make_shared<Message6>();
            mes->my_login = _status->getLogin();
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());
            this->getMess();
        }
    }
}


void interaction_chat::list_chat_P(){
    while (_status->getMenuChat() == MENU_CHAT::LIST_CHAT_P)
    {
        //Сообщение существует, откроем чат
        if(_status->get_message_status() && _status->getMessageType() == 53){
            auto m53 = std::dynamic_pointer_cast<Message53>(_status->getMessage()); // забираем сообщение
            _status->set_message_status(false);
            //ответ и открытие чата
            std::pair<std::string, std::string> data = _II->show_list_chat_P(m53->list_chat_P, _status);

            if ( data.first == "" || data.second == ""){//пользователь хочет выйти
                _status->setMenuChat(MENU_CHAT::MENU_VOID);
                _status->set_message_status(false);
                return;
            }
            else // пользователь выбрал чат, запрашиваем данные
            {
                // запросить данные приватного чата
                std::shared_ptr<Message8> mes = std::make_shared<Message8>();
                mes->user_sender = this->_status->getLogin();
                mes->user_recipient = data.first;
                json jj;
                mes->to_json(jj);
                _network->sendMess(jj.dump());
                this->getMess();

                // открываем приватный чат
                _status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
                chat_P();
            }
        }else//Не получали сообщение список пользователей, запрашиваем
        {
            //Запрос на получение списока чатов
            std::shared_ptr<Message5> mes = std::make_shared<Message5>();
            mes->my_login = this->_status->getLogin();
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());
            this->getMess();
        }
    }
}


void interaction_chat::chat_P(){
    // Данные уже должны были получить, если их нет, то там ошибка ответа
    while (_status->getMenuChat() == MENU_CHAT::SHOW_CHAT_P)
    {
        //Сообщение существует, откроем чат
        if(_status->get_message_status() && _status->getMessageType() == 52){
            auto m52 = std::dynamic_pointer_cast<Message52>(_status->getMessage()); // забираем сообщение
            _status->set_message_status(false);
            //ответ и открытие чата
            std::pair<std::string, std::string> fr_Us = m52->login_name_friend;
            std::shared_ptr<Message3> data = _II->show_chat_P(m52->history_chat_P, fr_Us.first, fr_Us.second, _status);

            if ( data->mess == ""){//пользователь хочет выйти
                _status->setMenuChat(MENU_CHAT::MENU_VOID);
                _status->set_message_status(false);
                return;
            }
            else // пользователь отправляет сообщение
            {
                // Отправка сообщения в приватный чат
                std::shared_ptr<Message3> mes = std::make_shared<Message3>();
                mes->user_sender = this->_status->getLogin();
                mes->mess = data->mess;
                mes->user_recipient = fr_Us.first;
                json jj;
                mes->to_json(jj);
                _network->sendMess(jj.dump());
                this->getMess();
            }
        }else//Не получали сообщение приватного чата
        {
            // обновить данные приватного чата
            std::shared_ptr<Message8> mes = std::make_shared<Message8>();
            mes->user_sender = _status->getLogin();
            json j;
            mes->to_json(j);
            _network->sendMess(j.dump());
            this->getMess();
        }
    }
}