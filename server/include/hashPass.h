#pragma once
#include<string>

//слабый хеш, будет разным на разных компиляторах, для БД SQL НЕ ИСПОЛЬЗОВАТЬ! в последующем заменить на SHA-256
std::string hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}