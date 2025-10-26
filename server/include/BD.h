#pragma once
#include "User.h" 
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include "Message.h"


class DataBase
{
private:

public:
        DataBase(){}
        virtual ~DataBase() = default;

        //=================================================================
        // БД: запись и чтение структур
        //=================================================================

        //запись юзера
        virtual void write_User(std::shared_ptr<User> user) = 0;


        //Поиск юзера пологину
        virtual std::shared_ptr<User> search_User(const std::string& log) = 0;


        // Получить список всех юзеров исключая юзера, который делает запрос
        virtual std::vector<std::pair<std::string, std::string>> list_all_User(std::string my_login) = 0;


        //получение бзеров с кем есть приватный чат, возвращает логины и имена для отображения
        // std::vector<std::pair<std::string ЛОГИН, std::string ИМЯ>>
        virtual std::vector<std::pair<std::string, std::string>> my_chat_P(std::string my_login) = 0;


        /*=====================================
                МЕТОДЫ ЧАТОВ
        =====================================*/

        //запись в приватный чат (Отправитель, получатель, сообщение)
        virtual bool write_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, const std::string& mess) = 0;


        // Загрузить историю приватного чата: пары <login, сообщение>
        virtual bool load_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, std::vector<MessageStruct>& out) = 0;


        //запись в общий чат, проверить перед записью существоваие файла!
        virtual bool write_Chat_H(std::shared_ptr<User> user_sender, const std::string& mess) = 0;


        // Загрузить историю общего чата: пары <login, сообщение>
        virtual bool load_Chat_H(std::vector<MessageStruct>& out) = 0;

        /*=====================================
                ПОЛЯ BAN / DISCON
        =====================================*/

        // Проверка ban по логину. Возвращает флаг ошибки: true — была ошибка, false — успешно.
        // Результат поля ban возвращается через isBanned по ссылке.
        virtual bool checkBanByLogin(const std::string& login, bool& isBanned) = 0;

        // Установка ban по логину. Возвращает флаг ошибки: true — была ошибка, false — успешно.
        virtual bool setBanByLogin(const std::string& login, bool banValue) = 0;

        // Аналогично для discon
        virtual bool checkDisconByLogin(const std::string& login, bool& isDiscon) = 0;
        virtual bool setDisconByLogin(const std::string& login, bool disconValue) = 0;

        // ADMIN: Получение списков забаненных и разлогированных юзеров
        virtual bool getBanAndDisconLists(std::vector<std::string>& banList, std::vector<std::string>& disconList) = 0;

};


