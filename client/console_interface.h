#pragma once
#include <string>
#include <vector>
#include <utility>
#include <Message.h>
#include "User.h"
#include <memory>
#include "interactive_interface.h"
#include "UserStatus.h"


// Цветовые макросы
#define _RED     "\x1B[31m"             // Красный
#define _GREEN   "\x1B[32m"             // Зеленый
#define _YELLOW  "\x1B[33m"             // Желтый
#define _BLUE    "\x1B[34m"             // Голубой
#define _MAGENTA "\x1B[35m"             // Фиолетовый
#define _CYAN    "\x1B[36m"             // Берюзовый
#define _CLEAR   "\033[0m"              // Очистить - Белый

#define _GREY_BG "\033[3;100;30m"       // Серый фон


class console_interface : public interactive_interface
{
private:

public:
    console_interface();
    ~console_interface();

    // отобразить чаты предать историю и 
    std::shared_ptr<Message3> show_chat_P(const std::vector<std::pair<std::string, std::string>>& history_chat_P, 
        const  std::string& login_friend_User, const  std::string& name_friend_User,
        std::shared_ptr<UserStatus> status) override;

    std::shared_ptr<Message4> show_chat_H(const  std::vector<std::vector<std::string>>& history_chat_H, 
        std::shared_ptr<UserStatus> status) override;

    // отобразить поле авторизации
    std::shared_ptr<Message1> authorization(std::shared_ptr<UserStatus> status) override;

    // отобразить поле регистрации логин
    std::shared_ptr<Message2> reg(std::shared_ptr<UserStatus> status) override;

    // отобразить список приватных чатов
    std::pair<std::string, std::string> show_list_chat_P(std::vector<std::pair<std::string, 
        std::string>>& list_Chat_P,
        std::shared_ptr<UserStatus> status) override;

    // отобразить пользователей кому написать
    std::pair<std::string, std::string> show_list_users(std::vector<std::pair<std::string, 
        std::string>>& list_Users,
        std::shared_ptr<UserStatus> status) override;

    //отобразить меню
    void show_chat_menu(std::shared_ptr<UserStatus> status) override;

    //отобразить авторизации
    void show_menu_authorization(std::shared_ptr<UserStatus> status) override;

    //отобразить авторизации
    void no_connect(std::shared_ptr<UserStatus> status) override;

    //для отображения системных сообщений
    void display_message(const std::string& info) override;
};


