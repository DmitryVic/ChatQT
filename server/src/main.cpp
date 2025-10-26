#include "NetworkServer.h"
#include "BD.h"
#include "BD_MySQL.h"
#include "Message.h"
#include "Core.h"
#include <iostream>
#include <unistd.h>             //базовые функции для работы с системой Linux
#include <string.h>             //библиотека для работы со строками C
#include <sys/socket.h>         //для работы с сокетами
#include <netinet/in.h>         //содержит структуры и константы для работы с протоколами
#include <nlohmann/json.hpp>
#include <memory>
#include <fstream>
#include <filesystem>
#include "Logger.h"
#include <exception>
#include "Logger.h"

using namespace std;
using json = nlohmann::json;

#define PORT 7777


int main() {
    
    // Универсальная настройка локали
    setlocale(LC_ALL, "ru_RU.UTF-8");

    // Для Linux
    #ifdef SET_GLOBAL_LOCALE_LINUX
    try {
        std::locale::global(std::locale("ru_RU.UTF-8"));
    } catch (const std::exception& e) {
        get_logger() << "Locale error: " << e.what() << "\n";
        std::locale::global(std::locale("C.UTF-8"));
    }
    #endif

    try {
        
        get_logger() << "Старт сервера (main) ...\n";

        std::shared_ptr<NetworkServer> network = std::make_shared<NetworkServer>(PORT);
        network->start();
        chat_start(network);

    } catch (const exception& e) {
        get_logger() << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}