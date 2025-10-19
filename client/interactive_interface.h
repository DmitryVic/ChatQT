#pragma once
#include <string>
#include <vector>
#include <utility>
#include <Message.h>
#include "User.h"
#include <memory>
#include "UserStatus.h"



class interactive_interface
{
private:
    /* data */
public:
    interactive_interface(){};
    virtual ~interactive_interface() = default;

    // отобразить чаты предать историю и 
    virtual std::shared_ptr<Message3> show_chat_P(const std::vector<std::pair<std::string, std::string>>& history_chat_P, 
        const  std::string& login_friend_User, const  std::string& name_friend_User,
        std::shared_ptr<UserStatus> status) = 0;
        
    virtual std::shared_ptr<Message4> show_chat_H(const  std::vector<std::vector<std::string>>& history_chat_H, 
        std::shared_ptr<UserStatus> status) = 0;

    // отобразить поле авторизации
    virtual  std::shared_ptr<Message1> authorization(std::shared_ptr<UserStatus> status) = 0;

    // отобразить поле регистрации логин
    virtual  std::shared_ptr<Message2> reg(std::shared_ptr<UserStatus> status) = 0;

    // отобразить список приватных чатов
    virtual  std::pair<std::string, std::string> show_list_chat_P(
        std::vector<std::pair<std::string, std::string>>& list_Chat_P,
        std::shared_ptr<UserStatus> status) = 0;

    // отобразить пользователей кому написать
    virtual  std::pair<std::string, std::string> show_list_users(
        std::vector<std::pair<std::string, std::string>>& list_Users,
        std::shared_ptr<UserStatus> status) = 0;

    //отобразить меню
    virtual void show_chat_menu(std::shared_ptr<UserStatus> status) = 0;

    //отобразить авторизации
    virtual void show_menu_authorization(std::shared_ptr<UserStatus> status) = 0;

    //сообщение о потере соединения
    virtual void no_connect(std::shared_ptr<UserStatus> status) = 0;

    //для отображения системных сообщений
    virtual void display_message(const  std::string& info) = 0;
};

