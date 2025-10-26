#include "BD_MySQL.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <mysql/mysql.h>
#include "Logger.h"
#include "Message.h"
#include "sha1.h"


DataBaseMySQL::DataBaseMySQL()
{
    mysql_init(&sql_mysql);

    // Подключение
    if (!mysql_real_connect(&sql_mysql, "localhost", this->SQL_USER, this->SQL_PASS, this->SQL_BD, 0, NULL, 0)) {
        get_logger() << "Ошибка подключения БД (MySQL): " << mysql_error(&sql_mysql);
        throw "Ошибка подключения БД (MySQL) - исключение конструктора";
    }
    get_logger() << "Подключение БД (MySQL) успешно!";

    // Кодировка
    mysql_set_character_set(&sql_mysql, "utf8mb4");
    mysql_query(&sql_mysql, "SET NAMES utf8mb4 COLLATE utf8mb4_unicode_ci"); // Для корректного отображения смайлов и других символов
    get_logger() << "Кодировка БД (MySQL): " << mysql_character_set_name(&sql_mysql);

    // Создание таблицы users
    if (mysql_query(&sql_mysql, "CREATE TABLE IF NOT EXISTS users(id INT AUTO_INCREMENT PRIMARY KEY, login VARCHAR(30) UNIQUE NOT NULL, name VARCHAR(30), surname VARCHAR(30), email VARCHAR(50) UNIQUE, ban BOOLEAN DEFAULT FALSE, discon BOOLEAN DEFAULT FALSE);") != 0) {
        throw "Ошибка БД (MySQL) создания таблицы users: " + std::string(mysql_error(&sql_mysql));
    }
    // Создание таблицы user_passwords
    if (mysql_query(&sql_mysql, "CREATE TABLE IF NOT EXISTS user_passwords(user_id INT NOT NULL, PRIMARY KEY (user_id), FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE, pass VARCHAR(255) NOT NULL);") != 0) {
        throw "Ошибка БД (MySQL) оздания таблицы user_passwords: " + std::string(mysql_error(&sql_mysql));
    }

    // Создание таблицы chats
    if (mysql_query(&sql_mysql, "CREATE TABLE IF NOT EXISTS chats(id INT AUTO_INCREMENT PRIMARY KEY, type ENUM('private','common') NOT NULL);") != 0) {
        throw "Ошибка БД (MySQL) оздания таблицы chats: " + std::string(mysql_error(&sql_mysql));
    }

    // Создание таблицы Связи пользователей с чатами user_chats
    if (mysql_query(&sql_mysql, "CREATE TABLE IF NOT EXISTS user_chats(user_id INT NOT NULL, chat_id INT NOT NULL, FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE, FOREIGN KEY (chat_id) REFERENCES chats(id) ON DELETE CASCADE, PRIMARY KEY (user_id, chat_id));") != 0) {
        throw "Ошибка БД (MySQL) оздания таблицы user_chats: " + std::string(mysql_error(&sql_mysql));
    }

    // Создание таблицы messages
    if (mysql_query(&sql_mysql, "CREATE TABLE IF NOT EXISTS messages(id INT AUTO_INCREMENT PRIMARY KEY, chat_id INT NOT NULL, FOREIGN KEY (chat_id) REFERENCES chats(id) ON DELETE CASCADE, sender_id INT NOT NULL, FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE, created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, text TEXT NOT NULL, recipient_id INT NULL,  FOREIGN KEY (recipient_id) REFERENCES users(id) ON DELETE SET NULL);") != 0) {
        throw "Ошибка БД (MySQL) оздания таблицы messages: " + std::string(mysql_error(&sql_mysql));
    }

    //создание чата общего если его нет
    std::string request_mysql =  "SELECT id FROM chats WHERE type = 'common';";
    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) не прошел запрос получения информации общего чата при инициализации БД: " << mysql_error(&sql_mysql);
        throw "Ошибка БД (MySQL) не прошел запрос получения информации общего чата при инициализации БД: " + std::string(mysql_error(&sql_mysql));
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            throw std::runtime_error(mysql_error(&sql_mysql));
        }
        // Нет результатов создаем общий чат
        request_mysql =  "INSERT INTO chats (type) VALUES ('common');";
        if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
            get_logger() << "Ошибка БД (MySQL) создания общего чата: " << mysql_error(&sql_mysql);
            throw "Ошибка БД (MySQL) создания общего чата: " + std::string(mysql_error(&sql_mysql));
        }
    }
    else{
        sql_row = mysql_fetch_row(sql_res);
        if (!sql_row) { // Нет подходящих записей ДОП ПРОВЕРКА
            mysql_free_result(sql_res);
            sql_res = nullptr;
            request_mysql =  "INSERT INTO chats (type) VALUES ('common');";
            if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
                get_logger() << "Ошибка БД (MySQL) создания общего чата: " << mysql_error(&sql_mysql);
                throw "Ошибка БД (MySQL) создания общего чата: " + std::string(mysql_error(&sql_mysql));
            }
        }
        else
        {
            // Проверка количества полей
            unsigned num_fields = mysql_num_fields(sql_res);
            if (num_fields != 1) {
                mysql_free_result(sql_res);
                sql_res = nullptr;
                throw std::runtime_error("Ожидалось 1 поле в результате получения информации общего чата при инициализации БД");
            }
            // Освобождаем, даже если данные корректны
            mysql_free_result(sql_res);
            sql_res = nullptr;
            // НЕТ ПРОВЕРКИ НА НЕСКОЛЬКО ПОДОБНЫХ ЧАТОВ В БД
        }
    }

    // ========= Создание пользователя admin, если его нет =========
    // Проверяем существование пользователя admin
    std::string check_admin = "SELECT id FROM users WHERE login = 'admin' LIMIT 1;";
    if (mysql_query(&sql_mysql, check_admin.c_str()) != 0) {
        throw std::runtime_error("Ошибка проверки существования admin: " + std::string(mysql_error(&sql_mysql)));
    }

    sql_res = mysql_store_result(&sql_mysql);
    if (!sql_res) {
        throw std::runtime_error("Ошибка получения результата проверки admin");
    }

    if (mysql_num_rows(sql_res) == 0) {
        mysql_free_result(sql_res);
        sql_res = nullptr;

        // Создаем пользователя admin
        std::string create_admin_user = "INSERT INTO users (login, name) VALUES ('admin', 'Administrator');";
        if (mysql_query(&sql_mysql, create_admin_user.c_str()) != 0) {
            throw std::runtime_error("Ошибка создания пользователя admin: " + std::string(mysql_error(&sql_mysql)));
        }

        // Добавляем пароль для admin
        std::string admin_pass = hashToString(sha1("admin")); // Используем такой же метод хеширования как в write_User
        std::string create_admin_pass = 
            "INSERT INTO user_passwords (user_id, pass) "
            "SELECT id, '" + escapeString(admin_pass) + "' "
            "FROM users WHERE login = 'admin';";
            
        if (mysql_query(&sql_mysql, create_admin_pass.c_str()) != 0) {
            throw std::runtime_error("Ошибка создания пароля для admin: " + std::string(mysql_error(&sql_mysql)));
        }
    } else {
        mysql_free_result(sql_res);
        sql_res = nullptr;
    }
}


DataBaseMySQL::~DataBaseMySQL(){
    if (sql_res) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
    }
    // Закрытие соединения
    mysql_close(&sql_mysql);
    get_logger() << "БД (MySQL) закрываю соединение!";
}


//=================================================================
// БД: запись и чтение структур
//=================================================================


//запись юзера
void DataBaseMySQL::write_User(std::shared_ptr<User> user) {
    // запись в таблицу users
    std::string request_mysql =  
    "INSERT INTO users (login, name) VALUES (\"" 
    + escapeString(user->getLogin())
    + "\", \""
    + escapeString(user->getName())
    + "\");";

    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) запись в таблицу users: " << mysql_error(&sql_mysql);
        throw "Ошибка БД (MySQL) запись в таблицу users: " + std::string(mysql_error(&sql_mysql));
    }

    // запись в таблицу user_passwords
    request_mysql =  
    "INSERT INTO user_passwords (user_id, pass) SELECT id, \"" 
    + escapeString(user->getPass())
    + "\" FROM users WHERE login = \""
    + escapeString(user->getLogin())
    + "\";";

    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) запись в таблицу user_passwords: " << mysql_error(&sql_mysql);
        throw "Ошибка БД (MySQL) запись в таблицу user_passwords: " + std::string(mysql_error(&sql_mysql));
    }
}


//Поиск юзера пологину
std::shared_ptr<User> DataBaseMySQL::search_User(const std::string& log){
    std::string request_mysql =  
    "SELECT u.login, up.pass, u.name "
    "FROM users u "
    "JOIN user_passwords up ON u.id = up.user_id "
    "WHERE u.login = '"+ escapeString(log) + "';";

    // Выполнение запроса
    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка поиска пользователя в БД (MySQL): " << mysql_error(&sql_mysql);
        throw "Ошибка поиска пользователя в БД (MySQL): " + std::string(mysql_error(&sql_mysql));
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            throw std::runtime_error(mysql_error(&sql_mysql));
        }
        // Нет результатов
        return nullptr;
    }

    sql_row = mysql_fetch_row(sql_res);
    if (!sql_row) { // Нет подходящих записей
        mysql_free_result(sql_res);
        sql_res = nullptr;
        return nullptr;
    }


    // Проверка количества полей
    unsigned num_fields = mysql_num_fields(sql_res);
    if (num_fields != 3) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
        throw std::runtime_error("Ожидалось 3 поля в результате поиска пользователя в БД (MySQL)");
    }

    // Безопасное преобразование
    std::string login = sql_row[0] ? sql_row[0] : "";
    std::string pass = sql_row[1] ? sql_row[1] : "";
    std::string name = sql_row[2] ? sql_row[2] : "";

    mysql_free_result(sql_res);
    sql_res = nullptr;
    return std::make_shared<User>(login, pass, name);
}


// Получить список всех юзеров исключая юзера, который делает запрос (и исключая admin)
std::vector<std::pair<std::string, std::string>> DataBaseMySQL::list_all_User(std::string my_login){
    
    std::vector<std::pair<std::string, std::string>> send;
    // Исключаем пользователя, который делает запрос, и системного администратора 'admin'
    std::string request_mysql =  "SELECT u.login, u.name FROM users u WHERE u.login NOT IN ('" + escapeString(my_login) + "', '" + escapeString(std::string("admin")) + "');";
    
    // Выполнение запроса
    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) запроса выдачи всех пользователей: " << mysql_error(&sql_mysql);
        throw "Ошибка БД (MySQL) запроса выдачи всех пользователей: " + std::string(mysql_error(&sql_mysql));
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            throw std::runtime_error(mysql_error(&sql_mysql));
        }
        // Нет результатов
        return send;
    }

    // Количество полей
    unsigned num_fields = mysql_num_fields(sql_res);
    if (num_fields != 2) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
        throw std::runtime_error("БД (MySQL) Ожидалось 2 поля в результате запроса выдачи всех пользователей");
    }

    while ((sql_row = mysql_fetch_row(sql_res))) {
        send.push_back({sql_row[0], sql_row[1]});
    }

    mysql_free_result(sql_res);
    sql_res = nullptr;
    return send;
}


//получение бзеров с кем есть приватный чат, возвращает логины и имена для отображения
// std::vector<std::pair<std::string ЛОГИН, std::string ИМЯ>>
std::vector<std::pair<std::string, std::string>> DataBaseMySQL::my_chat_P(std::string my_login) {

    std::vector<std::pair<std::string, std::string>> out;

        std::string request_mysql = 
        "SELECT us.login, us.name "
        "FROM users us "
        "JOIN user_chats usc ON usc.user_id = us.id "
        "WHERE usc.chat_id IN ("
            "SELECT uc.chat_id "
            "FROM user_chats uc "
            "WHERE user_id = "
                "(SELECT id FROM users u "
                "WHERE u.login = '"
                + escapeString(my_login) + 
                "' LIMIT 1) ) "
                "and usc.user_id  <> (SELECT id FROM users u WHERE u.login = '" 
                + escapeString(my_login) + 
                "' LIMIT 1);";

    // Выполнение запроса
    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) получения истории переписки my_chat_P: " << mysql_error(&sql_mysql);
        return out;
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            get_logger() << "Ошибка БД (MySQL) получения истории переписки my_chat_P (запрос пуст с ошибкой): " << mysql_error(&sql_mysql);
        }
        return out;
    }

    // Количество полей
    unsigned num_fields = mysql_num_fields(sql_res);
    if (num_fields != 2) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
        get_logger() << "БД (MySQL) Ожидалось 2 поля в результате зполучения истории переписки my_chat_P";
        return out;
    }

    while ((sql_row = mysql_fetch_row(sql_res))) {
        out.push_back({sql_row[0], sql_row[1]});
    }
    mysql_free_result(sql_res);
    sql_res = nullptr;
    return out;
}


/*=====================================
        ФУНКЦИИ ЧАТОВ
=====================================*/


//запись в приватный чат (Отправитель, получатель, сообщение)
bool DataBaseMySQL::write_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, const std::string& mess) {
    
    unsigned int chat_id = 0;
    bool transaction_ok = true;

    // Начало транзакции
    if (mysql_query(&sql_mysql, "START TRANSACTION")) {
        get_logger() << "Ошибка начала транзакции write_Chat_P: " << mysql_error(&sql_mysql);
        return false;
    }

    try {
        // Поиск существующего чата
        std::string request_mysql = 
            "WITH id2 AS ("
                "SELECT uc.chat_id, COUNT(uc.chat_id) as c2 "
                "FROM user_chats uc "
                "JOIN users u ON uc.user_id = u.id "
                "WHERE u.login IN ('" 
                + escapeString(user_sender->getLogin()) + "', '" + 
                escapeString(user_recipient->getLogin()) + "') "
                "GROUP BY uc.chat_id) "
            "SELECT chat_id FROM id2 WHERE c2 = 2 LIMIT 1";

        if (mysql_query(&sql_mysql, request_mysql.c_str())) {
            throw std::runtime_error("Ошибка поиска чата write_Chat_P: " + std::string(mysql_error(&sql_mysql)));
        }

        sql_res = mysql_store_result(&sql_mysql);
        if (!sql_res) throw std::runtime_error("Ошибка получения результата write_Chat_P");

        // Если чат не найден - создаем новый
        if (mysql_num_rows(sql_res) == 0) {
            mysql_free_result(sql_res);
            sql_res = nullptr;
            // Создаем новый чат
            std::string create_chat = 
                "INSERT INTO chats (type) VALUES ('private')";
            if (mysql_query(&sql_mysql, create_chat.c_str())) {
                throw std::runtime_error("Ошибка создания чата write_Chat_P: " + std::string(mysql_error(&sql_mysql)));
            }
            
            chat_id = mysql_insert_id(&sql_mysql);
            
            // Добавляем пользователей в чат
            std::string add_users = 
                "INSERT INTO user_chats (chat_id, user_id) VALUES "
                    "(" + std::to_string(chat_id) + ", (SELECT id FROM users WHERE login = '" + 
                        escapeString(user_sender->getLogin()) + "')), "
                    "(" + std::to_string(chat_id) + ", (SELECT id FROM users WHERE login = '" + 
                        escapeString(user_recipient->getLogin()) + "'))";

            if (mysql_query(&sql_mysql, add_users.c_str())) {
                throw std::runtime_error("Ошибка добавления в чат write_Chat_P: " + std::string(mysql_error(&sql_mysql)));
            }
        } else {
            // Получаем существующий chat_id
            sql_row = mysql_fetch_row(sql_res);
            chat_id = std::stoul(sql_row[0]);
            mysql_free_result(sql_res);
            sql_res = nullptr;
        }

        // Добавляем сообщение
        std::string insert_message = 
            "INSERT INTO messages (chat_id, sender_id, recipient_id, text) "
            "VALUES (" + std::to_string(chat_id) + ", "
            "(SELECT id FROM users WHERE login = '" + escapeString(user_sender->getLogin()) + "'), "
            "(SELECT id FROM users WHERE login = '" + escapeString(user_recipient->getLogin()) + "'), "
            "'" + escapeString(mess) + "')";

        if (mysql_query(&sql_mysql, insert_message.c_str())) {
            throw std::runtime_error("Ошибка добавления сообщения write_Chat_P: " + std::string(mysql_error(&sql_mysql)));
        }

        // Фиксация транзакции
        if (mysql_query(&sql_mysql, "COMMIT")) {
            throw std::runtime_error("Ошибка коммита write_Chat_P: " + std::string(mysql_error(&sql_mysql)));
        }

    } catch (const std::exception& e) {
        get_logger() << "Ошибка write_Chat_P: " << e.what();
        mysql_query(&sql_mysql, "ROLLBACK");
        transaction_ok = false;
    }

    // Очистка ресурсов
    if (sql_res) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
    }
    
    return transaction_ok;

}


// Загрузить историю приватного чата
bool DataBaseMySQL::load_Chat_P(std::shared_ptr<User> user_sender, std::shared_ptr<User> user_recipient, std::vector<MessageStruct>& out) {

    // Очистка предыдущих результатов
    while (mysql_next_result(&sql_mysql) == 0) {
        sql_res = mysql_store_result(&sql_mysql);
        if (sql_res) {mysql_free_result(sql_res); }
    }
    sql_res = nullptr;

    std::string request_mysql = 
    "SELECT uss.login, uss.name, m.text, m.created_at "
    "FROM messages m "
    "JOIN users uss ON uss.id = m.sender_id "
    "WHERE m.chat_id IN ("
    " SELECT chat_id FROM ("
    " SELECT uc.chat_id, COUNT(uc.chat_id) AS c2 "
    " FROM user_chats uc "
    " JOIN users u ON uc.user_id = u.id "
    " WHERE u.login IN ('"
    + escapeString(user_sender->getLogin()) +
    "', '"
    + escapeString(user_recipient->getLogin()) +
    "') "
    " GROUP BY uc.chat_id"
    " ) AS id2 "
    " WHERE c2 = 2"
    ") "
    "ORDER BY m.created_at";

    if (mysql_query(&sql_mysql, request_mysql.c_str())) {
        get_logger() << "Ошибка поиска чата load_Chat_P: " << mysql_error(&sql_mysql);
        out.clear();
        return false;
        //throw std::runtime_error("Ошибка поиска чата load_Chat_P: " + std::string(mysql_error(&sql_mysql)));
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            get_logger() << "Ошибка получения истории приватного чата БД (MySQL) load_Chat_P: " << mysql_error(&sql_mysql);
            return false;
        }
        // Нет результатов
        out.clear();
        return true;
    }

    // Количество полей
    unsigned num_fields = mysql_num_fields(sql_res);
    if (num_fields != 4) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
        get_logger() << "БД (MySQL) Ожидалось 4 поля в результате запроса получения истории приватного чата";
        return false;
    }

    while ((sql_row = mysql_fetch_row(sql_res))) {
        MessageStruct mesST;
        mesST.userLogin = sql_row[0];
        mesST.userName = sql_row[1];
        mesST.mess = sql_row[2];
        mesST.time = stringToTimestamp(sql_row[3]);
        out.emplace_back(mesST);
    }
    mysql_free_result(sql_res);
    sql_res = nullptr;
    return true;
}


//запись в общий чат, проверить перед записью существоваие файла!
bool DataBaseMySQL::write_Chat_H(std::shared_ptr<User> user_sender, const std::string& mess) {
    // получаем приватный чат ORDER BY id для надежности взять верхний (минимальный) а в методе отображения минимальный отображается
    std::string request_mysql = 
    "INSERT INTO messages (chat_id, sender_id, text) "
    "VALUES ("
    "(SELECT min(id) FROM chats WHERE type = 'common' ORDER BY id LIMIT 1),"
    "(SELECT u.id FROM users u WHERE u.login = '"
    + escapeString(user_sender->getLogin()) + "' LIMIT 1), '" 
    + escapeString(mess) + "');";
    // Выполнение запроса
    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка записи БД (MySQL) общего чата в write_Chat_H: " << mysql_error(&sql_mysql);
        return false;
    }
    return true;
}


// Загрузить историю общего чата
bool DataBaseMySQL::load_Chat_H(std::vector<MessageStruct>& out) {

    std::string request_mysql = 
    "SELECT u.login, u.name, m.text, m.created_at "
    "FROM messages m "
    "JOIN users u ON u.id = m.sender_id "
    "WHERE m.chat_id IN ("
        "SELECT min(id) as id_chat_p FROM chats WHERE type = 'common') " 
        "ORDER BY m.created_at;";

    if (mysql_query(&sql_mysql, request_mysql.c_str()) != 0) {
        get_logger() << "Ошибка полученияистории общего чата БД (MySQL) load_Chat_H: " << mysql_error(&sql_mysql);
        return false;
    }

    // Получение результата
    sql_res = mysql_store_result(&sql_mysql);
    
    if (!sql_res) { // Проверка на NULL результата
        if (mysql_errno(&sql_mysql)) {
            // Ошибка выполнения запроса
            get_logger() << "Ошибка получения истории общего чата БД (MySQL) load_Chat_H: " << mysql_error(&sql_mysql);
            return false;
        }
        // Нет результатов
        out.clear();
        return true;
    }

    // Количество полей
    unsigned num_fields = mysql_num_fields(sql_res);
    if (num_fields != 4) {
        mysql_free_result(sql_res);
        sql_res = nullptr;
        get_logger() << "БД (MySQL) Ожидалось 4 поля в результате запроса получения истории общего чата";
        return false;
    }

    while ((sql_row = mysql_fetch_row(sql_res))) {
        MessageStruct mesST;
        mesST.userLogin = sql_row[0];
        mesST.userName = sql_row[1];
        mesST.mess = sql_row[2];
        mesST.time = stringToTimestamp(sql_row[3]);
        out.emplace_back(mesST);
    }
    mysql_free_result(sql_res);
    sql_res = nullptr;
    return true;
}


        /*=====================================
                ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
        =====================================*/


std::string DataBaseMySQL::escapeString(const std::string& str) {
	char* escaped = new char[str.length() * 2 + 1];
	mysql_real_escape_string(&sql_mysql, escaped, str.c_str(), str.length());
	std::string result(escaped);
	delete[] escaped;
	return result;
}


/*=====================================
        РАБОТА С ПОЛЯМИ ban / discon
=====================================*/

bool DataBaseMySQL::checkBanByLogin(const std::string& login, bool& isBanned) {
    isBanned = false;

    std::string request = "SELECT ban FROM users WHERE login = '" + escapeString(login) + "' LIMIT 1;";
    if (mysql_query(&sql_mysql, request.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) checkBanByLogin: " << mysql_error(&sql_mysql);
        return true; // ошибка
    }

    MYSQL_RES* res = mysql_store_result(&sql_mysql);
    if (!res) {
        if (mysql_errno(&sql_mysql)) {
            get_logger() << "Ошибка получения результата checkBanByLogin: " << mysql_error(&sql_mysql);
            return true;
        }
        // нет результатов — считаем ошибкой (пользователь не найден)
        return true;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return true; // пользователь не найден
    }

    // row[0] может быть "0" или "1" или NULL
    if (row[0]) {
        std::string val = row[0];
        isBanned = (val == "1" || val == "true");
    } else {
        isBanned = false;
    }

    mysql_free_result(res);
    return false; // без ошибки
}

bool DataBaseMySQL::setBanByLogin(const std::string& login, bool banValue) {
    std::string request = "UPDATE users SET ban = " + std::to_string(banValue ? 1 : 0) + " WHERE login = '" + escapeString(login) + "' ;";
    if (mysql_query(&sql_mysql, request.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) setBanByLogin: " << mysql_error(&sql_mysql);
        return true; // ошибка выполнения запроса
    }

    my_ulonglong affected = mysql_affected_rows(&sql_mysql);
    if (affected == 0) {
        // ДОПОЛНИТЕЛЬНО Возможно, строка не найдена или значение не изменилось. Проверим существование пользователя
        std::string check = "SELECT id FROM users WHERE login = '" + escapeString(login) + "' LIMIT 1;";
        if (mysql_query(&sql_mysql, check.c_str()) != 0) {
            get_logger() << "Ошибка БД (MySQL) setBanByLogin (проверка существования): " << mysql_error(&sql_mysql);
            return true;
        }
        MYSQL_RES* res = mysql_store_result(&sql_mysql);
        if (!res) {
            // ошибка выполнения запроса или нет результатов
            if (mysql_errno(&sql_mysql)) {
                get_logger() << "Ошибка БД (MySQL) setBanByLogin (store_result): " << mysql_error(&sql_mysql);
                return true;
            }
            return true; // нет результатов
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (!row) {
            // пользователь не найден
            return true;
        }
        // пользователь существует, но affected_rows == 0 (значение могло не измениться) — считаем успешно
    }

    return false; // успешно
}

bool DataBaseMySQL::checkDisconByLogin(const std::string& login, bool& isDiscon) {
    isDiscon = false;

    std::string request = "SELECT discon FROM users WHERE login = '" + escapeString(login) + "' LIMIT 1;";
    if (mysql_query(&sql_mysql, request.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) checkDisconByLogin: " << mysql_error(&sql_mysql);
        return true; // ошибка
    }

    MYSQL_RES* res = mysql_store_result(&sql_mysql);
    if (!res) {
        if (mysql_errno(&sql_mysql)) {
            get_logger() << "Ошибка получения результата checkDisconByLogin: " << mysql_error(&sql_mysql);
            return true;
        }
        // нет результатов — считаем ошибкой (пользователь не найден)
        return true;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return true; // пользователь не найден
    }

    if (row[0]) {
        std::string val = row[0];
        isDiscon = (val == "1" || val == "true");
    } else {
        isDiscon = false;
    }

    mysql_free_result(res);
    return false; // без ошибки
}

bool DataBaseMySQL::setDisconByLogin(const std::string& login, bool disconValue) {
    std::string request = "UPDATE users SET discon = " + std::to_string(disconValue ? 1 : 0) + " WHERE login = '" + escapeString(login) + "' ;";
    if (mysql_query(&sql_mysql, request.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) setDisconByLogin: " << mysql_error(&sql_mysql);
        return true; // ошибка выполнения запроса
    }

    my_ulonglong affected = mysql_affected_rows(&sql_mysql);
    if (affected == 0) {
        // Возможно, строка не найдена или значение не изменилось. Проверим существование пользователя.
        std::string check = "SELECT id FROM users WHERE login = '" + escapeString(login) + "' LIMIT 1;";
        if (mysql_query(&sql_mysql, check.c_str()) != 0) {
            get_logger() << "Ошибка БД (MySQL) setDisconByLogin (проверка существования): " << mysql_error(&sql_mysql);
            return true;
        }
        MYSQL_RES* res = mysql_store_result(&sql_mysql);
        if (!res) {
            if (mysql_errno(&sql_mysql)) {
                get_logger() << "Ошибка БД (MySQL) setDisconByLogin (store_result): " << mysql_error(&sql_mysql);
                return true;
            }
            return true;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (!row) {
            // пользователь не найден
            return true;
        }
        // пользователь существует, affected_rows == 0 -> значение прежнее, считаем успешно
    }

    return false; // успешно
}

// ADMIN: Получение списка всех пользователей с информацией ban/discon
bool DataBaseMySQL::getBanAndDisconLists(std::vector<AdminDataUsers>& listDataUser) {
    listDataUser.clear();

    // Получаем всех пользователей и их флаги ban/discon
    std::string request = "SELECT login, name, ban, discon FROM users;";

    if (mysql_query(&sql_mysql, request.c_str()) != 0) {
        get_logger() << "Ошибка БД (MySQL) getBanAndDisconLists: " << mysql_error(&sql_mysql);
        return true; // ошибка
    }

    MYSQL_RES* res = mysql_store_result(&sql_mysql);
    if (!res) {
        if (mysql_errno(&sql_mysql)) {
            get_logger() << "Ошибка получения результата getBanAndDisconLists: " << mysql_error(&sql_mysql);
            return true;
        }
        // нет результатов — пустой список
        return false;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        AdminDataUsers info;
        info.userLogin = row[0] ? row[0] : "";
        info.userName = row[1] ? row[1] : "";

        bool isBan = false;
        bool isDiscon = false;
        if (row[2]) {
            std::string val = row[2];
            isBan = (val == "1" || val == "true");
        }
        if (row[3]) {
            std::string val = row[3];
            isDiscon = (val == "1" || val == "true");
        }

        info.banStatus = isBan;
        // discon == true означает разлогирован; online = !discon
        info.onlineStatus = !isDiscon;

        listDataUser.push_back(std::move(info));
    }

    mysql_free_result(res);
    return false; // успешно
}