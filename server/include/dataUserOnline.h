#pragma once
#include "BD.h"
#include "BD_MySQL.h"
#include <string>
#include <memory>

struct dataUserOnline
{
    std::string online_user_login;
    int client_socket;
    std::unique_ptr<DataBase> db;
    std::string recv_buffer_;
};

// У каждого потока своя копия
extern thread_local dataUserOnline currentUser;