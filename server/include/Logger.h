#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>  // std::ostringstream 


class Logger
{
private:
    static constexpr size_t MAX_FILE_SIZE = 1 * 1024 * 1024;  // 1 МБ
    
    std::string logFile = "log.txt";
    std::string logFileMessUsers = "logUsers.txt";
    std::string logFilePath = "file/";

    std::ofstream logStream;
    std::ofstream logStreamMessUsers;

    std::shared_mutex sh_mtx_log;
    std::shared_mutex mtx_log_users;
    
    void checkAndRotateLog(const std::string& filename);
    
public:
    Logger();
    ~Logger();
    void log(const std::string& message);
    void logMessageUser(const std::string& message);
    std::string getLastLineLog();
    std::string getLastLineLogUsers();

    template <typename T>
    Logger& operator<<(const T& message)
    {
        std::ostringstream oss;     // для каждого потока свое
        oss << message;             // формируем строку
        log(oss.str());             // потокобезопасная запись
        return *this;
    }
};


 // Доступ к логеру 
Logger& get_logger();