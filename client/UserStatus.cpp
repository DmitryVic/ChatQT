#include "UserStatus.h"
#include "User.h"
#include <string>
#include "MessageHandler.h"
#include <memory>
#include "Message.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>


UserStatus::UserStatus() : myUser{"", "", ""} {}



//##################################################
// Обновление UI
//##################################################

// есть ли обновления ? _uiUpdatePending
bool UserStatus::hasUpdatePending() const{
    return this->_uiUpdatePending.load();
}
// очистить флаг обновления
void UserStatus::clearUpdatePending(){
    this->_uiUpdatePending.store(false);
}


// Уведомить UI о необходимости обновления (вызывается из рабочих потоков)
void UserStatus::resetUI() {
    this->_uiUpdatePending.store(true);
}   

//##################################################
// Системные 
//##################################################

// Проверка состояния приложения
bool UserStatus::running() const { return this->_isRunning.load(); }

// Остановка приложения
void UserStatus::stopApp() { this->_isRunning.store(false); }

//##################################################
// Очереди сообщений Сервер - Клиент
//##################################################

// Методы для работы с очередью принятых сообщений
void UserStatus::pushAcceptedMessage(const std::string& msg) {
    {
        std::lock_guard<std::mutex> lock(acceptQueueMutex);
        messageQueueAccept.push(msg);
    }
    acceptQqueueCondVar.notify_one();
}

std::string UserStatus::popAcceptedMessage() {
    std::lock_guard<std::mutex> lock(acceptQueueMutex);
    if (messageQueueAccept.empty()) {
        return "";
    }
    std::string msg = messageQueueAccept.front();
    messageQueueAccept.pop();
    return msg;
}
bool UserStatus::hasAcceptedMessages() const {
    std::lock_guard<std::mutex> lock(acceptQueueMutex);
    return !messageQueueAccept.empty();
}

  // Ожидание и получение сообщения
std::string UserStatus::waitAndPopAcceptedMessage() {
    std::unique_lock<std::mutex> lock(acceptQueueMutex);
    while (_isRunning.load() && this->getNetworckConnect()) {
        if (!messageQueueAccept.empty()) {
            std::string msg = messageQueueAccept.front();
            messageQueueAccept.pop();
            return msg;
        }
        acceptQqueueCondVar.wait_for(lock, std::chrono::milliseconds(500));
    }
    // Проверяем еще раз после выхода из цикла
    if (!messageQueueAccept.empty()) {
        std::string msg = messageQueueAccept.front();
        messageQueueAccept.pop();
        return msg;
    }
    return "";
}

// Методы для работы с очередью сообщений на отправку
void UserStatus::pushMessageToSend(const std::string& msg) {
    {
        std::lock_guard<std::mutex> lock(sendQueueMutex);
        messageQueueSend.push(msg);
    }
    sendQueueCondVar.notify_one();
}

std::string UserStatus::popMessageToSend() {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    if (messageQueueSend.empty()) {
        return "";
    }
    std::string msg = messageQueueSend.front();
    messageQueueSend.pop();
    return msg;
}

bool UserStatus::hasMessagesToSend() const {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    return !messageQueueSend.empty();
}

std::string UserStatus::waitAndPopMessageToSend() {
    std::unique_lock<std::mutex> lock(sendQueueMutex);
    while (_isRunning.load() && this->getNetworckConnect()) {
        if (!messageQueueSend.empty()) {
            std::string msg = messageQueueSend.front();
            messageQueueSend.pop();
            return msg;
        }
        sendQueueCondVar.wait_for(lock, std::chrono::milliseconds(500));
    }
    // Проверяем еще раз после выхода из цикла
    if (!messageQueueSend.empty()) {
        std::string msg = messageQueueSend.front();
        messageQueueSend.pop();
        return msg;
    }
    return "";
}

//##################################################
// флаги
//##################################################

//получить флаг наличия критических ошибок на сервере (true - ошибки)
bool UserStatus::getSrvStatErrFatall() const{
    return _srvStatErrFatall.load(std::memory_order_acquire);
}

//изменить статус флага наличия критических ошибок на сервере (true - ошибки)
void UserStatus::setSrvStatErrFatall(bool SrvStatErrFatall){
    _srvStatErrFatall.store(SrvStatErrFatall, std::memory_order_release);
    this->resetUI();
}

//##################################################
// СЕТЬ 
//##################################################

//получить флаг подкключчения к сети (true - подключены)
bool UserStatus::getNetworckConnect() const{
    return _networckConnect.load(std::memory_order_acquire);
}

//изменить статус флага подкключчения к сети (true - подключены)
void UserStatus::setNetworckConnect(bool networckConnect){
    _networckConnect.store(networckConnect, std::memory_order_release);
    this->resetUI();
}


//получить флаг запуска потоков сети - true - запущены
bool UserStatus::getNetworckThreadsSost() const{
    return _networckThreadsSost.load(std::memory_order_acquire);
}

//изменить статус флага запуска потоков сети - true - запущены
void UserStatus::setNetworckThreadsSost(bool networckThreadsSost){
    _networckThreadsSost.store(networckThreadsSost, std::memory_order_release);
    this->resetUI();
}



//##################################################
// СООБЩЕНИЯ
//##################################################

std::vector<MessageStruct> UserStatus::getMessList() const{
    std::lock_guard<std::mutex> lock(_messListMutex);
    return this->_messList;
}

void UserStatus::setMessList(std::vector<MessageStruct> &&messList){
    std::lock_guard<std::mutex> lock(_messListMutex);
    this->_messList = messList;
    this->resetUI();
}

std::string UserStatus::getChatName() const{
    std::lock_guard<std::mutex> lock(_chatNameMutex);
    return this->_chatName;
}

void UserStatus::setChatName(std::string &&chatName){
    std::lock_guard<std::mutex> lock(_chatNameMutex);
    this->_chatName = chatName;
    this->resetUI();
}

FriendData UserStatus::getFriendOpenChatP() const{
    std::lock_guard<std::mutex> lock(_friendOpenChatPMutex);
    return this->_friendOpenChatP;
}

void UserStatus::setFriendOpenChatP(FriendData &&friendD){
    std::lock_guard<std::mutex> lock(_friendOpenChatPMutex);
    this->_friendOpenChatP = friendD;
    this->resetUI();
}

chat UserStatus::getChatOpen() const{
  std::lock_guard<std::mutex> lock(_chatOpenMutex);
  return this->_chatOpen;
}
void UserStatus::setChatOpen(chat chatOpen){
  std::lock_guard<std::mutex> lock(_chatOpenMutex);
  this->_chatOpen = chatOpen;
  //this->resetUI(); // обновление и так будет при принятом сообщении
}

// Сообщения обновлены?
bool UserStatus::getResetMess() const{
    return _resetMess.load(std::memory_order_acquire);
}
    // Сообщения обновлены?
void UserStatus::setResetMess(bool resetMess){
    _resetMess.store(resetMess, std::memory_order_release);
}

//##################################################
//#################### СПИСКИ ####################
//##################################################

//pair<us.login, us.name>
std::vector<std::pair<std::string, std::string>> UserStatus::getListChatP() const{
    std::lock_guard<std::mutex> lock(_list_chat_P_Mutex);
    return this->_list_chat_P;
}

//pair<us.login, us.name>
void UserStatus::setListChatP(std::vector<std::pair<std::string, std::string>> &&listChatP){
    std::lock_guard<std::mutex> lock(_list_chat_P_Mutex);
    this->_list_chat_P = listChatP;
    this->resetUI();
}

//pair<us.login, us.name>
std::vector<std::pair<std::string, std::string>> UserStatus::getListUsers() const{
    std::lock_guard<std::mutex> lock(_list_Users_Mutex);
    return this->_list_Users;
}

//pair<us.login, us.name>
void UserStatus::setListUsers(std::vector<std::pair<std::string, std::string>> &&listUsers){
    std::lock_guard<std::mutex> lock(_list_Users_Mutex);
    this->_list_Users = listUsers;
    this->resetUI();
}


//##################################################
// Уведомления
//##################################################

std::string UserStatus::getNotifi() const{
    std::lock_guard<std::mutex> lock(notifiMutex);
    return notifi;
}

void UserStatus::setNotifi(std::string notifi){
    std::lock_guard<std::mutex> lock(notifiMutex);
    this->notifi = notifi;
    this->resetUI();
}


//##################################################
// Данные пользователя - АВТОРИЗАЦИЯ / РЕГИСТРАЦИЯ
//##################################################

void UserStatus::setUser(User user){
    std::lock_guard<std::mutex> lock(_myUserMutex);
    this->myUser = user;
}

User UserStatus::getUser() const{
    std::lock_guard<std::mutex> lock(_myUserMutex);
    return this->myUser;
}

    //получить флаг авторизации
bool UserStatus::getAuthorizationStatus() const{
        return _authorizationStatus.load(std::memory_order_acquire);
}

//изменить статус флага авторизация
void UserStatus::setAuthorizationStatus(bool authorizationStatus){
    _authorizationStatus.store(authorizationStatus, std::memory_order_release);
}

//получить флаг логин занят
bool UserStatus::getLoginBusy() const{
    return _loginBusy.load(std::memory_order_acquire);
}

//изменить статус флага логин занят
void UserStatus::setLoginBusy(bool loginBusy){
    _loginBusy.store(loginBusy, std::memory_order_release);
}   


//Ответ от сетвера получен
bool UserStatus::getServerResponseReg() const{
    return _serverResponseReg.load(std::memory_order_acquire);
}
//Ответ от сетвера получен
void UserStatus::setServerResponseReg(bool serverResponse){
    _serverResponseReg.store(serverResponse, std::memory_order_release);
}