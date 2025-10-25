#include "Logger.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <sys/stat.h>  // mkdir
#include <filesystem>

namespace fs = std::filesystem;

Logger::Logger()
{
    // Создаем директорию для логов, если её нет
    struct stat info;  
    if (stat(logFilePath.c_str(), &info) != 0) {  
        // Проверить, существует ли директория  
        if (mkdir(logFilePath.c_str(), 0777) != 0) {  
            // Создать директорию  
            throw "Ошибка создания каталога Logger";  
        }  
    } else if (!(info.st_mode & S_IFDIR)) {  
        throw "Путь существует, но это не каталог, ошибка создания каталога Logger";  
    }
    // Открываем файл для логов
    logStream.open(logFilePath + logFile, std::ios::app);
    if (!logStream.is_open())
        throw "Ошибка открытия файла логов Logger";
    
    // Открываем файл для логов сообщений пользователей
    logStreamMessUsers.open(logFilePath + logFileMessUsers, std::ios::app);
    if (!logStreamMessUsers.is_open())
        throw "Ошибка открытия файла логов сообщений пользователей Logger";
    
}


Logger::~Logger()
{
    if (logStream.is_open())
        logStream.close();
    if (logStreamMessUsers.is_open())
        logStreamMessUsers.close();
}

// Если размер файла превышает MAX_FILE_SIZE, выполняем ротацию
// ограничение на количество файлов: log.txt - 10, logUsers.txt - 3
// метод самм удаляет старые файлы и переименовывает текущий
void Logger::checkAndRotateLog(const std::string& filename) {
    std::string fullPath = logFilePath + filename;
    if (fs::exists(fullPath)) {
        size_t fileSize = fs::file_size(fullPath);
        if (fileSize >= MAX_FILE_SIZE) {
            if (logStream.is_open()) logStream.close();
            if (logStreamMessUsers.is_open()) logStreamMessUsers.close();

            // Определяем максимальное количество файлов в зависимости от типа лога
            int maxFiles = (filename == logFile) ? 10 : 3;
            
            // Удаляем самый старый файл, если он существует
            std::string oldestFile = fullPath + "." + std::to_string(maxFiles);
            if (fs::exists(oldestFile)) {
                fs::remove(oldestFile);
            }

            // Сдвигаем существующие файлы
            for (int i = maxFiles - 1; i >= 1; --i) {
                std::string oldFile = fullPath + "." + std::to_string(i);
                std::string newFile = fullPath + "." + std::to_string(i + 1);
                if (fs::exists(oldFile)) {
                    fs::rename(oldFile, newFile);
                }
            }

            // Переименовываем текущий файл
            fs::rename(fullPath, fullPath + ".1");
            
            // Переоткрываем файл
            if (filename == logFile) {
                logStream.open(fullPath, std::ios::app);
            } else {
                logStreamMessUsers.open(fullPath, std::ios::app);
            }
        }
    }
}

void Logger::log(const std::string& message)
{
    std::unique_lock lock(sh_mtx_log);
    if (logStream.is_open())
    {
        checkAndRotateLog(logFile);
        
        // Получаем текущее время
        std::time_t now = std::time(nullptr);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        
        logStream << "[" << buf << "] " << message << std::endl;
        logStream.flush(); // Сразу записываем в файл
    }
}


void Logger::logMessageUser(const std::string& message)
{
    std::unique_lock lock(mtx_log_users);
    if (logStreamMessUsers.is_open())
    {
        checkAndRotateLog(logFileMessUsers);
        
        // Получаем текущее время
        std::time_t now = std::time(nullptr);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        
        logStreamMessUsers << "[" << buf << "] " << message << std::endl;
        logStreamMessUsers.flush(); // Сразу записываем в файл
    }
}


std::string Logger::getLastLineLog(){
    std::shared_lock lock(sh_mtx_log);
    std::ifstream file(logFilePath + logFile);
    if (!file.is_open())
        return ""; 
    std::string line;
    std::string lastLine;
    while (std::getline(file, line))
        lastLine = line;
    
    return lastLine;
}


std::string Logger::getLastLineLogUsers(){
    std::shared_lock lock(mtx_log_users);
    std::ifstream file(logFilePath + logFileMessUsers);
    if (!file.is_open())
        return ""; 
    std::string line;
    std::string lastLine;
    while (std::getline(file, line))
        lastLine = line;
    
    return lastLine;
}


Logger& get_logger(){
    static Logger logger;
    return logger;
}