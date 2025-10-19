#pragma once
#include <string>
#include "User.h"
#include <memory>
#include "UserStatus.h"
#include "NetworkClient.h"
#include "interactive_interface.h"
#include "MessageHandler.h"

class interaction_chat
{
private:
    std::shared_ptr<NetworkClient> _network;
    std::shared_ptr<interactive_interface> _II;
    std::shared_ptr<MessageHandler> _hendl;
    std::shared_ptr<UserStatus> _status;
    void menu_chat();
    void getMess();
    void chat_H();
    void list_user();
    void list_chat_P();
    void chat_P();
public:
    interaction_chat(std::shared_ptr<NetworkClient> network, 
                   std::shared_ptr<interactive_interface> II, 
                   std::shared_ptr<MessageHandler> hendl,
                   std::shared_ptr<UserStatus> status);
    
    ~interaction_chat() = default;
    
    void start();
    
};