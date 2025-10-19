#pragma once
#include "User.h" 
#include "Message.h"
#include "BD.h"
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <mysql/mysql.h>



class DataBaseMySQL : public DataBase
{
private:
        const char* SQL_USER = "chat_user";
        const char* SQL_PASS = "12345678";
        const char* SQL_BD = "chat";
        MYSQL sql_mysql;
        MYSQL_RES* sql_res = nullptr;
        MYSQL_ROW sql_row;

public:
        DataBaseMySQL();
        ~DataBaseMySQL();

        //=================================================================
        // БД: запись и чтение структур
        //=================================================================

        //запись юзера
        void write_User(std::shared_ptr<User> user) override;


        //Поиск юзера пологину
        std::shared_ptr<User> search_User(const std::string& log) override;


        // Получить список всех юзеров исключая юзера, который делает запрос
        std::vector<std::pair<std::string, std::string>> list_all_User(std::string my_login) override;


        //получение бзеров с кем есть приватный чат, возвращает логины и имена для отображения
        // std::vector<std::pair<std::string ЛОГИН, std::string ИМЯ>>
        std::vector<std::pair<std::string, std::string>> my_chat_P(std::string my_login) override;


        /*=====================================
                МЕТОДЫ ЧАТОВ
        =====================================*/

        //запись в приватный чат (Отправитель, получатель, сообщение)
        bool write_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, const std::string& mess) override;


        // Загрузить историю приватного чата: пары <login, сообщение>
        bool load_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, std::vector<MessageStruct>& out) override;


        //запись в общий чат, проверить перед записью существоваие файла!
        bool write_Chat_H(std::shared_ptr<User> user_sender, const std::string& mess) override;


        // Загрузить историю общего чата
        bool load_Chat_H(std::vector<MessageStruct>& out) override;

        /*=====================================
                ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
        =====================================*/

        //Экранирование
        std::string escapeString(const std::string& str);

};



