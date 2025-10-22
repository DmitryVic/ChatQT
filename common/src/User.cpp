#include "User.h"


// Получить логин
std::string User::getLogin() const{
    return this->_login;
}


// Получить имя
std::string User::getName() const{
    return this->_name;
}


// Получить проль не безопасный
std::string User::getPass() const{
    return this->_pasword;
}


// Задать логин
void User::setLogin(std::string& login){
    this->_login = login;
}

// Задать имя
void User::setName(std::string& name){
        this->_name = name;
}

// Задать проль не безопасный
void User::setPass(std::string& pasword){
    this->_pasword = pasword;
}