#pragma once
#include "User.h"
#include <string>
#include <memory>
#include "Message.h"
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>
#include <condition_variable>
#include <chrono>

/*

ОСНОВНАЯ ЗАДАЧА UserStatus ХРАНИТЬ ИНФОРМАЦИЮ О ТЕКУЩЕМ СТАТУСЕ КЛИЕНТА
ПРЕДОСТАВЛЯЯ ОСТАЛЬНЫМ КОМПОНЕНТАМ МЕНЯТЬ И ПОЛУЧАТЬ ЗНАЧЕНИЯ
ОБЕСПЕЧИТЬ ПОТОКОБЕЗОПАСНЫЙ ОБМЕН

*/

struct FriendData
{
    std::string name;
    std::string login;
};

// Добавить потом личку
// TO DO
enum chat {PRIVATE_CHAT, SHARED_CHAT};

class UserStatus
{
private:
    
    //#################### Обновление UI ####################    
    // Флаг, указывающий, что обновление UI запланировано (чтобы избежать множественных вызовов)
    std::atomic<bool> _uiUpdatePending {false};

    //#################### Системные ####################
    //false - завершение работы (UI закрыт)
    std::atomic<bool> _isRunning {true};

    //#################### Очереди сообщений сервер - клиент ####################
    // Потокобезопасная очередь принятых сообщений
    std::queue<std::string> messageQueueAccept;
    mutable std::mutex acceptQueueMutex;
    std::condition_variable acceptQqueueCondVar;

    // Потокобезопасная очередь сообщений на отправку
    std::queue<std::string> messageQueueSend;
    mutable std::mutex sendQueueMutex;
    std::condition_variable sendQueueCondVar;

     //#################### Флаги ####################

    //              --- Статусы ---
    //наличие критических ошибок на сервере  (true - ошибки)
    std::atomic<bool> _srvStatErrFatall {false};
    

    //#################### СЕТЬ ####################
    //Подключен к сети true - подключен
    std::atomic<bool> _networckConnect {false};
    //потоки принятия/отправки запущены? true - запущены
    std::atomic<bool> _networckThreadsSost {false};

    //#################### СООБЩЕНИЯ ####################
    // одна структура (что отображается в окне сообщений)
    std::vector<MessageStruct> _messList;
    mutable std::mutex _messListMutex;
    // название чата (что отображается в окне сообщений)
    std::string _chatName;
    mutable std::mutex _chatNameMutex;

    FriendData _friendOpenChatP;
    mutable std::mutex _friendOpenChatPMutex;

    chat _chatOpen = SHARED_CHAT;
    mutable std::mutex _chatOpenMutex;

    // Сообщения обновлены?
    std::atomic<bool> _resetMess {false};

    //#################### СПИСКИ ####################
    
    //pair<us.login, us.name>
    std::vector<std::pair<std::string, std::string>> _list_chat_P;
    mutable std::mutex _list_chat_P_Mutex;
    //pair<us.login, us.name>
    std::vector<std::pair<std::string, std::string>> _list_Users;
    mutable std::mutex _list_Users_Mutex;

    //#################### Уведомления ####################
    mutable std::mutex notifiMutex;
    std::string notifi;
    
    //#################### Данные пользователя - АВТОРИЗАЦИЯ / РЕГИСТРАЦИЯ ####################
    User myUser;
    mutable std::mutex _myUserMutex;
    //Авторизация выполненна или нет, от сервера должны получить true + свой логин с именем 
    std::atomic<bool> _authorizationStatus = false;

    //Логин занят
    std::atomic<bool> _loginBusy = false;

    //Ответ от сетвера получен
     std::atomic<bool> _serverResponseReg = false;
    
public:
    UserStatus();
    ~UserStatus() = default;

    //#################### Обновление UI ####################
    // есть ли обновления ?
    bool hasUpdatePending() const;
    // очистить флаг обновления
    void clearUpdatePending();

    // Уведомить UI о необходимости обновления (вызывается из рабочих потоков)
    void resetUI();
    
    //#################### Системные ####################
    void stopApp();
    bool running() const;


    //#################### Очереди сообщений сервер - клиент ####################

    // Методы для работы с очередью принятых сообщений

    // Добавляет сообщение в очередь
    void pushAcceptedMessage(const std::string& msg); 
    // Возвращает пустую строку, если очередь пуста
    std::string popAcceptedMessage(); 
    // Проверка наличия сообщений
    bool hasAcceptedMessages() const; 
    // Ожидание и получение сообщения
    std::string waitAndPopAcceptedMessage();

    // Методы для работы с очередью сообщений на отправку
    
    // Добавляет сообщение в очередь
    void pushMessageToSend(const std::string& msg);
    // Возвращает пустую строку, если очередь пуста
    std::string popMessageToSend();
    // Проверка наличия сообщений
    bool hasMessagesToSend() const;
    // Ожидание и получение сообщения
    std::string waitAndPopMessageToSend();

    //#################### Флаги ####################

    //получить флаг наличия критических ошибок на сервере (true - ошибки) 
    bool getSrvStatErrFatall() const;
    //изменить статус флага наличия критических ошибок на сервере (true - ошибки)
    void setSrvStatErrFatall(bool SrvStatErrFatall);

    //#################### СЕТЬ ####################
    
    //получить флаг подкключчения к сети (true - подключены)
    bool getNetworckConnect() const;

    //изменить статус флага подкключчения к сети (true - подключены)
    void setNetworckConnect(bool networckConnect);
    
    //получить флаг запуска потоков сети - true - запущены
    bool getNetworckThreadsSost() const;

    //изменить статус флага запуска потоков сети - true - запущены
    void setNetworckThreadsSost(bool networckThreadsSost);


    //#################### СООБЩЕНИЯ ####################

    std::vector<MessageStruct> getMessList() const;
    void setMessList(std::vector<MessageStruct> &&messList);

    std::string getChatName() const;
    void setChatName(std::string &&chatName);

    FriendData getFriendOpenChatP() const;
    void setFriendOpenChatP(FriendData &&friendD);

    chat getChatOpen() const;
    void setChatOpen(chat chatOpen);

    // Сообщения обновлены?
    bool getResetMess() const;
     // Сообщения обновлены?
    void setResetMess(bool resetMess);

    //#################### СПИСКИ ####################
    
    //pair<us.login, us.name>
    std::vector<std::pair<std::string, std::string>> getListChatP() const;
    //pair<us.login, us.name>
    void setListChatP(std::vector<std::pair<std::string, std::string>> &&listChatP);

    //pair<us.login, us.name>
    std::vector<std::pair<std::string, std::string>> getListUsers() const;
    //pair<us.login, us.name>
    void setListUsers(std::vector<std::pair<std::string, std::string>> &&listUsers);


    //#################### Уведомления ####################
    std::string getNotifi() const;
    void setNotifi(std::string notifi);


    //#################### Данные пользователя - АВТОРИЗАЦИЯ / РЕГИСТРАЦИЯ ####################
    void setUser(User user);
    User getUser() const;

    //получить флаг авторизации
    bool getAuthorizationStatus() const;
    //изменить статус флага авторизация
    void setAuthorizationStatus(bool authorizationStatus);

    //получить флаг логин занят
    bool getLoginBusy() const;
    //изменить статус флага логин занят
    void setLoginBusy(bool loginBusy);

    //Ответ от сетвера получен
    bool getServerResponseReg() const;
    //Ответ от сетвера получен
    void setServerResponseReg(bool serverResponse);

};



