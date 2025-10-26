#include "Logger.h"
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <filesystem>

namespace fs = std::filesystem;

Logger::Logger()
{
    try {
        // Создаем директорию для логов, если её нет
        fs::create_directories(logFilePath);
        
        // Проверяем, что путь является директорией
        if (!fs::is_directory(logFilePath)) {
            throw std::runtime_error("Путь существует, но это не каталог");
        }
    } catch (const fs::filesystem_error& e) {
        throw std::string("Ошибка создания каталога Logger: ") + e.what();
    }
    
    // Открываем файл для логов
    logStream.open(logFilePath + logFile, std::ios::app);
    if (!logStream.is_open())
        throw "Ошибка открытия файла логов Logger";
}


Logger::~Logger()
{
    if (logStream.is_open())
        logStream.close();
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


Logger& get_logger(){
    static Logger logger;
    return logger;
}