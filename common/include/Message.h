#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <memory>
#include <utility>


using json = nlohmann::json;

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

/*=====================================
        СООБЩЕНИЯ ОТ СЕРВЕРА
=====================================*/


// Ответ сервера на запрос true or false
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
    std::vector<std::vector<std::string>> history_chat_H;
    
    int getTupe() const override { return 51; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Передача данных приватного чата
class Message52 : public Message {
public:
    std::vector<std::pair<std::string, std::string>> history_chat_P;
    std::pair<std::string, std::string> login_name_friend;
    std::pair<std::string, std::string> login_name_MY;
    
    int getTupe() const override { return 52; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// Передача списка истории приватных чатов
class Message53 : public Message {
public:
    std::vector<std::pair<std::string, std::string>> list_chat_P;
    
    int getTupe() const override { return 53; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};


// получить список всех юзеров в чате кому написать
class Message54 : public Message {
public:
    std::vector<std::pair<std::string, std::string>> list_Users;
    
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


// Ответ сервера вернуть имя
class Message56 : public Message {
public:
    std::string my_name;
    
    int getTupe() const override { return 56; }
    
    void to_json(json& j) const override;
    
    void from_json(const json& j) override;
};

// Из строки JSON в класс сообщения
std::shared_ptr<Message> parse_message(const std::string& json_str);


