#pragma once
#include "User.h"
#include <string>
#include <memory>
#include "Message.h"


/*

ОСНОВНАЯ ЗАДАЧА UserStatus ХРАНИТЬ ИНФОРМАЦИЮ О ТЕКУЩЕМ СТАТУСЕ КЛИЕНТА
ПРЕДОСТАВЛЯЯ ОСТАЛЬНЫМ КОМПОНЕНТАМ МЕНЯТЬ И ПОЛУЧАТЬ ЗНАЧЕНИЯ

*/

enum MENU_CHAT {LIST_CHAT_P, LIST_USERS, EXIT, SHOW_CHAT_H, SHOW_CHAT_P, MENU_VOID}; 


enum MENU_AUTHORIZATION {EXIT_PROGRAMM, AUTHORIZATION, REG, AUTHORIZATION_SUCCESSFUL, VOID_REG}; 




class UserStatus
{
private:
    MENU_CHAT menu_chat = MENU_CHAT::MENU_VOID;
    MENU_AUTHORIZATION menu_authoriz = MENU_AUTHORIZATION::VOID_REG;
    User myUser;

    bool message_status = false;
    std::shared_ptr<Message> message = nullptr;
    int typeMessage = 0;

public:
    UserStatus();
    ~UserStatus() = default;

    void setMenuChat(MENU_CHAT menu_chat);
    MENU_CHAT getMenuChat() const;

    void setMenuAuthoriz(MENU_AUTHORIZATION menu_authoriz);
    MENU_AUTHORIZATION getMenuAuthoriz() const;

    void setLogin(std::string login);
    std::string getLogin() const;

    void setPass(std::string pass);
    std::string getPass() const;

    void setName(std::string Name);
    std::string getName() const;

    int getMessageType() const;
    void setMessType(int type);

    bool get_message_status() const;
    void set_message_status(bool message_status);
    
    std::shared_ptr<Message> getMessage() const;
    void setMess(std::shared_ptr<Message> message);
};



