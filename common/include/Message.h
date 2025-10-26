#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <memory>
#include <utility>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ctime>

using json = nlohmann::json;


/*=====================================
        Обработка времени
=====================================*/
// Тип для хранения даты и времени
using Timestamp = std::chrono::system_clock::time_point;
// Функция для получения текущего времени
Timestamp getCurrentTimestamp();
// Конвертация из Timestamp в строку (MySQL формат)
std::string timestampToString(const Timestamp& ts);
// Конвертация из строки в Timestamp
Timestamp stringToTimestamp(const std::string& str);


/*=====================================
        Собственные типы данных
=====================================*/


//стрруктура сообщения
struct MessageStruct
{
    Timestamp time;
    std::string mess;
    std::string userName;
    std::string userLogin;
    
    // Метод для сериализации в JSON
    void to_json(json& j) const {
        j = {
            {"time", std::chrono::system_clock::to_time_t(time)},
            {"mess", mess},
            {"userName", userName},
            {"userLogin", userLogin}
        };
    }

    // Метод для десериализации из JSON
    void from_json(const json& j) {
        time = std::chrono::system_clock::from_time_t(j.at("time").get<std::time_t>());
        mess = j.at("mess").get<std::string>();
        userName = j.at("userName").get<std::string>();
        userLogin = j.at("userLogin").get<std::string>();
    }
};

// Регистрация типов для работы с JSON
namespace nlohmann {
    template<>
    struct adl_serializer<MessageStruct> {
        static void to_json(json& j, const MessageStruct& r) {
            r.to_json(j);
        }

        static void from_json(const json& j, MessageStruct& r) {
            r.from_json(j);
        }
    };

    template<>
    struct adl_serializer<Timestamp> {
        static void to_json(json& j, const Timestamp& ts) {
            j = timestampToString(ts);
        }

        static void from_json(const json& j, Timestamp& ts) {
            ts = stringToTimestamp(j.get<std::string>());
        }
    };
}


// Абстрактный базовый класс
class Message {
public:
    virtual int getTupe() const = 0; 
    //создание JSON
    virtual void to_json(json& j) const = 0;
    //  из JSON
    virtual void from_json(const json& j) = 0;
    virtual ~Message() = default;

    //Создание сообщений
    static std::unique_ptr<Message> create(int type);
    
};


/*=====================================
        СООБЩЕНИЯ ОТ КЛИЕНТА
=====================================*/


// Передавча логина и пароля
class Message1 : public Message {
public:
    std::string login;
    std::string pass;
    
    int getTupe() const override { return 1; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;

};


// Регистрация
class Message2 : public Message {
public:
    std::string login;
    std::string pass;
    std::string name;
    
    int getTupe() const override { return 2; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Отправка сообщения в приватный чат
class Message3 : public Message {
public:
    std::string user_sender;
    std::string user_recipient;
    std::string mess;
    
    int getTupe() const override { return 3; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Отправка сообщения в общий чат
class Message4 : public Message {
public:
    std::string login_user_sender;
    std::string name_user_sender;
    std::string mess;
    
    int getTupe() const override { return 4; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Запрос на получение списка приватных чатов
class Message5 : public Message {
public:
    std::string my_login;
    
    int getTupe() const override { return 5; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Запрос на получение списока всех юзеров в чате кому написать
class Message6 : public Message {
public:
    std::string my_login;
    
    int getTupe() const override { return 6; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Запрос юзера получить свое имя
class Message7 : public Message {
public:
    std::string my_login;
    
    int getTupe() const override { return 7; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// обновить данные приватного чата
class Message8 : public Message {
public:
    std::string user_sender;
    std::string user_recipient;
    
    int getTupe() const override { return 8; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// обновить данные общего чата
class Message9 : public Message {
public:
    std::string user_sender;
    
    int getTupe() const override { return 9; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// ADMIN discon user
class Message10 : public Message {
public:
    std::string user_login;
    int getTupe() const override { return 10; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

// ADMIN ban user
class Message11 : public Message {
public:
    std::string user_login;
    bool ban_value;
    int getTupe() const override { return 11; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

// ADMIN запрос на получение спика забаненных юзеров и разлогированных
class Message12 : public Message {
public:
    int getTupe() const override { return 12; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

/*=====================================
        СООБЩЕНИЯ ОТ СЕРВЕРА
=====================================*/


// false - ошибка сервера
class Message50 : public Message {
public:
    bool status_request;
    
    int getTupe() const override { return 50; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;

    void push_Mes_Err();
};


// Передача данных общего чата
class Message51 : public Message {
public:
    std::vector<MessageStruct> history_chat_H;
    
    int getTupe() const override { return 51; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Передача данных приватного чата
class Message52 : public Message {
public:
    std::vector<MessageStruct> history_chat_P;
    std::pair<std::string, std::string> login_name_friend;
    std::pair<std::string, std::string> login_name_MY;
    
    int getTupe() const override { return 52; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Передача списка истории приватных чатов (pair<us.login, us.name>)
class Message53 : public Message {
public:
    std::vector<std::pair<std::string, std::string>> list_chat_P;    //pair<us.login, us.name>
    
    int getTupe() const override { return 53; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// получить список всех юзеров чата (pair<us.login, us.name>)
class Message54 : public Message {
public:
    std::vector<std::pair<std::string, std::string>> list_Users;    //pair<us.login, us.name>
    
    int getTupe() const override { return 54; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Ответ сервера логин занят
class Message55 : public Message {
public:
    bool status_request;
    
    int getTupe() const override { return 55; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Ответ сервера Вы залогинены
class Message56 : public Message {
public:
    bool authorization;
    std::string my_name;
    std::string my_login;
    
    int getTupe() const override { return 56; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// ADMIN ответ на discon user
class Message57 : public Message {
public:
    bool status_request;
    int getTupe() const override { return 57; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

// ADMIN ответ на ban user
class Message58 : public Message {
public:
    bool status_request;
    int getTupe() const override { return 58; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

// ADMIN ответ на запрос списка забаненных и разлогированных юзеров
class Message59 : public Message {
public:
    std::vector<std::string> list_login_users_ban;
    std::vector<std::string> list_login_users_discon;
    int getTupe() const override { return 59; }
    void to_json(json& j) const override;
    void from_json(const json& j) override;
};

// Из строки JSON в класс сообщения
std::shared_ptr<Message> parse_message(const std::string& json_str);


