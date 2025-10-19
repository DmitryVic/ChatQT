#include "UserStatus.h"
#include "User.h"
#include <string>
#include "MessageHandler.h"
#include <memory>
#include "Message.h"



UserStatus::UserStatus() : myUser("", "", ""){}



void UserStatus::setMenuChat(MENU_CHAT menu_chat){
    this->menu_chat = menu_chat;
}


MENU_CHAT UserStatus::getMenuChat() const {
    return this->menu_chat;
}


void UserStatus::setMenuAuthoriz(MENU_AUTHORIZATION menu_authoriz){
    this->menu_authoriz = menu_authoriz;
}


MENU_AUTHORIZATION UserStatus::getMenuAuthoriz() const{
    return this->menu_authoriz;
}


void UserStatus::setLogin(std::string login){
    this->myUser.setLogin(login);
}


std::string UserStatus::getLogin() const{
    return this->myUser.getLogin();
}


void UserStatus::setPass(std::string pass){
    this->myUser.setPass(pass);
}


std::string UserStatus::getPass() const{
    return this->myUser.getPass();
}


void UserStatus::setName(std::string Name){
    this->myUser.setName(Name);
}


std::string UserStatus::getName() const{
    return this->myUser.getName();
}


int UserStatus::getMessageType() const{
    return this->typeMessage;
}


void UserStatus::setMessType(int type){
    this->typeMessage = type;
}


std::shared_ptr<Message> UserStatus::getMessage() const{
    return this->message;
}


void UserStatus::setMess(std::shared_ptr<Message> message){
    this->message = message;
}


bool UserStatus::get_message_status() const{
    return this->message_status;
}

// Сообщение существует?
void UserStatus::set_message_status(bool message_status){
    this->message_status = message_status;
}