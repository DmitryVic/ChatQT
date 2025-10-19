#pragma once
#include "BD.h"
#include "User.h" 
#include <string>
#include <vector>
#include <utility>
#include <memory>


class DataBaseFile : public DataBase
{
private:
        std::string CHAT_HARED_FILE = "file/ChH.txt";
        std::string BD_FILE = "file/DB.txt";
public:
        DataBaseFile();
        ~DataBaseFile();
        
        /*=====================================
        ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ДЛЯ РАБОТЫ С ФАЙЛАМИ
        =====================================*/

        // Файл существует или нет
        bool fileExists(const std::string& filename);
        // Создает директорию, если её нет
        void createDirectoryIfNeeded(const std::string& path);
        // Создать файл
        void createFile(const std::string& filename);

        // Список всех .txt-файлов в директории dir
        std::vector<std::string> listChatFiles(const std::string& dir);

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

        //вспомогательный метод собирает название чата из предосталенных логинов
        std::string fileNameChatP(std::string user1, std::string user2);

        //запись в приватный чат (Отправитель, получатель, сообщение)
        bool write_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, const std::string& mess) override;


        // Загрузить историю приватного чата: пары <login, сообщение>
        bool load_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, std::vector<std::pair<std::string, std::string>>& out) override;


        //запись в общий чат, проверить перед записью существоваие файла!
        bool write_Chat_H(std::shared_ptr<User> user_sender, const std::string& mess) override;


        // Загрузить историю общего чата: пары <login, сообщение>
        bool load_Chat_H(std::vector<std::vector<std::string>>& out) override;

};



