#include "Mediator.h"
#include <string>
#include <atomic>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

Mediator::Mediator()
{
}

Mediator::~Mediator()
{
    std::cerr << "Mediator DEL\n";
}


//##################################################
// Обновление UI
//##################################################

// регистрация уведомителя (должна вызываться из GUI при инициализации)
void Mediator::setUiNotifyCallback(std::function<void()> cb) {
    // thread-safe assignment (std::function присваивается атомарно только если никто не вызывает одновременно,
    // для простоты сделаем обычное присваивание — регистрация делается в инициализации GUI)
    _notifyCallback = std::move(cb);
}

// Сброс флага обновления - главный поток вызывает после обработки уведомления
void Mediator::clearUiUpdatePending() {
    _uiUpdatePending.store(false, std::memory_order_release);
}

// Уведомить UI о необходимости обновления (вызывается из рабочих потоков)
void Mediator::resetUI() {
    // помечаем, что UI нужно обновить и - единожды - вызываем callback
    if (_notifyCallback) {
        bool expected = false;
        if (_uiUpdatePending.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            // только если ранее не было "pending" — вызываем уведомление
            try {
                _notifyCallback(); // callback от UI
            } catch(...) { /* _______ */ }
        }
    }
}   


//##################################################
// Системные 
//##################################################

// Проверка состояния приложения
bool Mediator::running() const { return this->_isRunning.load(); }

// Остановка приложения
void Mediator::stopApp() { this->_isRunning.store(false); }


//##################################################
// Очереди сообщений Сервер - Клиент | Потокобезопасно ✅
//##################################################

// Методы для работы с очередью принятых сообщений
void Mediator::pushAcceptedMessage(const std::string& msg) {
    {
        std::lock_guard<std::mutex> lock(acceptQueueMutex);
        messageQueueAccept.push(msg);
    }
    acceptQqueueCondVar.notify_one();
}

std::string Mediator::popAcceptedMessage() {
    std::lock_guard<std::mutex> lock(acceptQueueMutex);
    if (messageQueueAccept.empty()) {
        return "";
    }
    std::string msg = messageQueueAccept.front();
    messageQueueAccept.pop();
    return msg;
}
bool Mediator::hasAcceptedMessages() const {
    std::lock_guard<std::mutex> lock(acceptQueueMutex);
    return !messageQueueAccept.empty();
}

  // Ожидание и получение сообщения
std::string Mediator::waitAndPopAcceptedMessage() {
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
void Mediator::pushMessageToSend(const std::string& msg) {
    {
        std::lock_guard<std::mutex> lock(sendQueueMutex);
        messageQueueSend.push(msg);
    }
    sendQueueCondVar.notify_one();
}

std::string Mediator::popMessageToSend() {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    if (messageQueueSend.empty()) {
        return "";
    }
    std::string msg = messageQueueSend.front();
    messageQueueSend.pop();
    return msg;
}

bool Mediator::hasMessagesToSend() const {
    std::lock_guard<std::mutex> lock(sendQueueMutex);
    return !messageQueueSend.empty();
}

std::string Mediator::waitAndPopMessageToSend() {
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
// флаги | Потокобезопасно ✅
//##################################################

//получить флаг наличия критических ошибок на сервере (true - ошибки)
bool Mediator::getSrvStatErrFatall() const{
    return _srvStatErrFatall.load(std::memory_order_acquire);
}

//изменить статус флага наличия критических ошибок на сервере (true - ошибки)
void Mediator::setSrvStatErrFatall(bool SrvStatErrFatall){
    _srvStatErrFatall.store(SrvStatErrFatall, std::memory_order_release);
    this->resetUI();
}

//получить флаг наличия НЕ критических ошибок на сервере (true - ошибки)
bool Mediator::getSrvStatErr() const{
    return _srvStatErr.load(std::memory_order_acquire);
}
//изменить статус флага наличия НЕ критических ошибок на сервере (true - ошибки)
void Mediator::setSrvStatErr(bool StatErr){
    _srvStatErr.store(StatErr, std::memory_order_release);
    this->resetUI();
}

//получить флаг наличия ошибок в обмене (true - ошибки)
bool Mediator::getSrvStatErrMess() const{
    return _srvStatErrMess.load(std::memory_order_acquire);
}
//изменить статус флага наличия ошибок в обмене (true - ошибки)
void Mediator::setSrvStatErrMess(bool StatErrMess){
    _srvStatErrMess.store(StatErrMess, std::memory_order_release);
    this->resetUI();
}


    //      --- Обработка отправки сообщения ---
//сообщение отправлено в очередь
bool Mediator::getSendMessStatus() const{
    return _send_mess_status.load(std::memory_order_acquire);
}
//сообщение отправлено в очередь НЕТ обновления UI
void Mediator::setSendMessStatus(bool sendMessStatus){
    _send_mess_status.store(sendMessStatus, std::memory_order_release);
    // this->resetUI();  НЕ требуется
}
// Сообщение сохранено сервером
bool Mediator::getSaveMessStatus() const{
    return _save_mess_status.load(std::memory_order_acquire);
}
// Сообщение сохранено сервером
void Mediator::setSaveMessStatus(bool statErrMessStatus){
    _save_mess_status.store(statErrMessStatus, std::memory_order_release);
    this->resetUI();
}

//##################################################
// СЕТЬ | Потокобезопасно ✅
//##################################################

//получить флаг подкключчения к сети (true - подключены)
bool Mediator::getNetworckConnect() const{
    return _networckConnect.load(std::memory_order_acquire);
}

//изменить статус флага подкключчения к сети (true - подключены)
void Mediator::setNetworckConnect(bool networckConnect){
    _networckConnect.store(networckConnect, std::memory_order_release);
    this->resetUI();
}


//получить флаг запуска потоков сети - true - запущены
bool Mediator::getNetworckThreadsSost() const{
    return _networckThreadsSost.load(std::memory_order_acquire);
}

//изменить статус флага запуска потоков сети - true - запущены
void Mediator::setNetworckThreadsSost(bool networckThreadsSost){
    _networckThreadsSost.store(networckThreadsSost, std::memory_order_release);
    this->resetUI();
}


//##################################################
// СТРУКТУРЫ ДАННЫХ | Потокобезопасно ✅
//##################################################




//##################################################
// Уведомления | Потокобезопасно ✅
//##################################################

std::string Mediator::getNotifi() const{
    std::lock_guard<std::mutex> lock(notifiMutex);
    return notifi;
}

void Mediator::setNotifi(std::string notifi){
    std::lock_guard<std::mutex> lock(notifiMutex);
    this->notifi = notifi;
    this->resetUI();
}