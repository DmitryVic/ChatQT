#include "console_interface.h"
#include <string>
#include <vector>
#include <utility>
#include <Message.h>
#include "User.h"
#include <memory>
#include "interactive_interface.h"
#include <iostream>
#include <iomanip>   // Для std::ws
using namespace std;


console_interface::console_interface()
{
    cout << _GREY_BG << "\nДобро пожаловать в Чат!\n" << _CLEAR << endl;
}

console_interface::~console_interface()
{
    cout << _GREY_BG << "\nДосвидания!\n" << _CLEAR << endl;
}


// отобразить чат P
std::shared_ptr<Message3> console_interface::show_chat_P(const std::vector<std::pair<std::string, std::string>>& history_chat_P, 
        const  std::string& login_friend_User, const  std::string& name_friend_User,
        std::shared_ptr<UserStatus> status){
        
    std::string userInput; // Вводимое пользователем знначение
    if (history_chat_P.empty())
    {
        cout << _GREY_BG << "\nИстория чата с ползователем " << name_friend_User << " пуста\n\n" << _CLEAR << endl;
    }
    else
    {
        cout << _GREY_BG << "\nЧат с ползователем " << name_friend_User << "\n\n" << _CLEAR << endl;
        for (auto mes : history_chat_P)
        {
            if (mes.first == status->getLogin())
            {
                cout << _CYAN << status->getName() << _CLEAR << "\t" << mes.second  << "\n" << endl;
            }
            else if (mes.first == login_friend_User)
            {
                cout << _YELLOW << name_friend_User << _CLEAR << "\t" << mes.second  << "\n" << endl;
            }
            
            else
            {
                cout << _MAGENTA << mes.first << "\t" << mes.second << _CLEAR << "\n" << endl;
            }
        }
    }
    
    cout << _MAGENTA<< "\nДля выхода введите /exit" << _GREEN << "\nОтправьте сообщение:" << _CLEAR << endl;

    std::getline(std::cin >> std::ws, userInput);  // Читаем всю строку
    std::shared_ptr<Message3> answer = std::make_shared<Message3>();
    answer->user_sender = status->getLogin();
    answer->user_recipient = login_friend_User;
    if (userInput != "/exit"){
        answer->mess = userInput;
        status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
    }
    else
    {
        answer->mess = "";
        status->setMenuChat(MENU_CHAT::LIST_CHAT_P);
    }
    return answer;
}

// отобразить чат H
std::shared_ptr<Message4> console_interface::show_chat_H(const  std::vector<std::vector<std::string>>& history_chat_H, 
        std::shared_ptr<UserStatus> status){
                
        std::string userInput; // Вводимое пользователем знначение
        if (history_chat_H.empty())
        {
            cout << _GREY_BG << "\nИстория общего чата пуста\n\n" << _CLEAR << endl;
        }
        else
        {
            cout << _GREY_BG << "\nОбщий чат" << "\n\n" << _CLEAR << endl;
            for (auto data : history_chat_H)
            {
                if (data[0] == status->getLogin())
                {
                    cout << _CYAN << data[1] << _CLEAR << "\t" << data[2]  << "\n" << endl;
                }
                else
                {
                    cout << _YELLOW << data[1] << _CLEAR << "\t" << data[2]  << "\n" << endl;
                }
                
            }
        }
        
        cout << _MAGENTA<< "\nДля выхода введите /exit" << _GREEN << "\nОтправьте сообщение:" << _CLEAR << endl;

        std::getline(std::cin >> std::ws, userInput);  // Читаем всю строку
        std::shared_ptr<Message4> answer = std::make_shared<Message4>();
        answer->login_user_sender = status->getLogin();
        answer->name_user_sender = status->getName();
        if (userInput != "/exit"){
            answer->mess = userInput;
            status->setMenuChat(MENU_CHAT::SHOW_CHAT_H);
        }
        else
        {
            answer->mess = "";
            status->setMenuChat(MENU_CHAT::MENU_VOID);
        }
        return answer;
}


// отобразить поле авторизации
std::shared_ptr<Message1> console_interface::authorization(std::shared_ptr<UserStatus> status) {
    std::string login;
    std::string pass;

    cout << _GREY_BG << "\nАвторизация\n\n" << _CLEAR << endl;
    
    cout << _GREEN << "\nВведите логин" << _CLEAR << endl;
    std::getline(std::cin, login);

    cout << _GREEN << "\nВведите пароль" << _CLEAR << endl;
    std::getline(std::cin, pass);

    std::shared_ptr<Message1> answer = std::make_shared<Message1>();
    answer->login = login;
    answer->pass = pass;
    status->setLogin(login);
    status->setPass(pass);
    return answer;
}


// отобразить поле регистрации логин
std::shared_ptr<Message2> console_interface::reg(std::shared_ptr<UserStatus> status) {
    std::string login;
    std::string pass;
    std::string name;

    cout << _GREY_BG << "\nРегистрация\n\n" << _CLEAR << endl;
    
    cout << _GREEN << "\nВведите логин" << _CLEAR << endl;
    std::getline(std::cin, login);

    cout << _GREEN << "\nВведите пароль" << _CLEAR << endl;
    std::getline(std::cin, pass);

    cout << _GREEN << "\nВведите имя" << _CLEAR << endl;
    std::getline(std::cin, name);

    std::shared_ptr<Message2> answer = std::make_shared<Message2>();
    answer->login = login;
    answer->pass = pass;
    answer->name = name;
    status->setLogin(login);
    status->setPass(pass);
    status->setName(name);
    return answer;
}


// отобразить список приватных чатов
std::pair<std::string, std::string> console_interface::show_list_chat_P(
    std::vector<std::pair<std::string, std::string>>& list_Chat_P,
    std::shared_ptr<UserStatus> status) {
    
    status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
    size_t userNamberInput = 9999;                                    // для получения номера пользователя и открытия чата
    
    while (true)
    {
        if (list_Chat_P.empty())
        {
            cout << _GREY_BG << "\nУ вас нет чатов" << _CLEAR << endl;
        }
        else
        {
            cout << _GREY_BG << "\nСписок чатов" << "\n\n" << _CLEAR << endl;
            int namber_chat = 1;
            for (auto chat : list_Chat_P)
                cout << "[" << namber_chat++ << "] \t" << chat.second << "\n" << endl;
        }
            
        cout << endl;
        cout    << _CYAN << " Меню:" << _CLEAR << endl
                << "0 - Назад;" << endl
                << "N - ID чата из списка, кому написать?" << endl
                << "Ведите значение" << endl;
        
        // Обработка ввода
        if (!(cin >> userNamberInput)) {
            cin.clear();                                                    // Сброс флагов ошибок
            cin.ignore(numeric_limits<streamsize>::max(), '\n');            // Очистка буфера
            cout << _YELLOW << "Ошибка: введите число." << _CLEAR << endl;
            userNamberInput = 9999;                                          // Предотвращаем возможное зацикливание
        }

        if (userNamberInput == 0){
            status->setMenuChat(MENU_CHAT::MENU_VOID);
            return {};
        }
        else if (userNamberInput > list_Chat_P.size())
        {
            cout << _YELLOW << "Ошибка: введите номер чата или 0 для выхода." << _CLEAR << endl;
            status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
        }
        else
        {
            status->setMenuChat(MENU_CHAT::SHOW_CHAT_P);
            return list_Chat_P[userNamberInput - 1];
        }
    }

}


// отобразить пользователей кому написать
std::pair<std::string, std::string> console_interface::show_list_users(std::vector<std::pair<std::string, 
    std::string>>& list_Users,
    std::shared_ptr<UserStatus> status) {

    status->setMenuChat(MENU_CHAT::LIST_USERS);
    size_t userNamberInput = 9999;                                    // для получения номера пользователя и открытия чата
    
    while (true)
    {
        if (list_Users.empty())
        {
            cout << _GREY_BG << "\nНету пользователей" << _CLEAR << endl;
        }
        else
        {
            cout << _GREY_BG << "\nСписок пользователей" << "\n\n" << _CLEAR << endl;
            int namber_chat = 1;
            for (auto chat : list_Users)
                cout << "[" << namber_chat++ << "] \t" << chat.second << "\n" << endl;
        }
            
        cout << endl;
        cout    << _CYAN << " Меню:" << _CLEAR << endl
                << "0 - Назад;" << endl
                << "N - Пользователя, кому написать?" << endl
                << "Ведите значение" << endl;
        
        // Обработка ввода
        if (!(cin >> userNamberInput)) {
            cin.clear();                                                    // Сброс флагов ошибок
            cin.ignore(numeric_limits<streamsize>::max(), '\n');            // Очистка буфера
            cout << _YELLOW << "Ошибка: введите число." << _CLEAR << endl;
            userNamberInput = 9999;                                          // Предотвращаем возможное зацикливание
        }

        if (userNamberInput == 0){
            status->setMenuChat(MENU_CHAT::MENU_VOID);
            return {{},{}};
        }
        else if (userNamberInput > list_Users.size())
        {
            cout << _YELLOW << "Ошибка: введите номер чата или 0 для выхода." << _CLEAR << endl;
            status->setMenuChat(MENU_CHAT::LIST_USERS);
        }
        else
        {
            status->setMenuChat(MENU_CHAT::LIST_USERS);
            return list_Users[userNamberInput - 1];
        }
    }
}


//отобразить меню
void console_interface::show_chat_menu(std::shared_ptr<UserStatus> status){
    std::string menu;

    while (true)
    {
        cout << _CYAN << " Меню:" << _CLEAR << endl
        << "\t0 - Выход" << endl
        << "\t1 - Открыть список приватных чатов" << endl
        << "\t2 - Список пользователей" << endl
        << "\t3 - Открыть общий чат" << endl
        << _GREEN << "Ведите значение" << _CLEAR << endl;

        std::getline(std::cin, menu);
        if (menu == "0")
        {
            status->setMenuChat(MENU_CHAT::EXIT);
            return;
        }
        else if (menu == "1")
        {
            status->setMenuChat(MENU_CHAT::LIST_CHAT_P);
            return;
        }
        else if (menu == "2")
        {
            status->setMenuChat(MENU_CHAT::LIST_USERS);
            return;
        }
        else if (menu == "3")
        {
            status->setMenuChat(MENU_CHAT::SHOW_CHAT_H);
            return;
        }
        else
        {
            cout << _YELLOW << "\nНе верно введено значение, попробуй еще раз" << _CLEAR << endl;
        }
    }
}


//отобразить авторизации
void console_interface::show_menu_authorization(std::shared_ptr<UserStatus> status) {
    std::string menu;

    while (true)
    {
        cout << "\nДля выбора введите значение."<< endl 
            << _CYAN << " Меню:" << _CLEAR << endl
            << "\t0 - Закрыть приложение;" << endl
            << "\t1 - Авторизация;" << endl
            << "\t2 - Регистрация" << endl
            << _GREEN << "Ведите значение" << _CLEAR << endl;

        std::getline(std::cin, menu);
        if (menu == "0")
        {
            status->setMenuAuthoriz(MENU_AUTHORIZATION::EXIT_PROGRAMM);
            return;
        }
        else if (menu == "1")
        {
            status->setMenuAuthoriz(MENU_AUTHORIZATION::AUTHORIZATION);
            return;
        }
        else if (menu == "2")
        {
            status->setMenuAuthoriz(MENU_AUTHORIZATION::REG);
            return;
        }
        else
        {
            cout << _YELLOW << "\nНе верно введено значение, попробуй еще раз" << _CLEAR << endl;
        }
    }

}


//отобразить авторизации
void console_interface::no_connect(std::shared_ptr<UserStatus> status) {
    cout << _MAGENTA << "Нет связи с сервером" << _CLEAR << endl;
    status->setMenuAuthoriz(MENU_AUTHORIZATION::EXIT_PROGRAMM);
}


//для отображения системных сообщений
void console_interface::display_message(const std::string& info) {
    cout << _GREY_BG << "\n" << info << "\n" << _CLEAR << endl;
}