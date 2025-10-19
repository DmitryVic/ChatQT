#pragma once
#include "Message.h"
#include <string>
#include <vector>
#include <atomic>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>


class Mediator
{
private:
    //#################### Обновление UI ####################    
    // Callback для уведомления UI о новых данных
    std::function<void()> _notifyCallback;
    // Флаг, указывающий, что обновление UI запланировано (чтобы избежать множественных вызовов)
    std::atomic<bool> _uiUpdatePending {false};

    //#################### Системные ####################
    //false - завершение работы (UI закрыт)
    std::atomic<bool> _isRunning = true;

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
    //наличие НЕ критических ошибок на сервере  (true - ошибки)
    std::atomic<bool> _srvStatErr {false};

    //наличие ошибок в обмене  (true - ошибки)
    std::atomic<bool> _srvStatErrMess {false};

    //      --- Обработка отправки сообщения ---
    //сообщение отправлено в очередь
    std::atomic<bool> _send_mess_status {false};
    // Сообщение сохранено сервером
    std::atomic<bool> _save_mess_status {false};

    //#################### СЕТЬ ####################
    //Подключен к сети true - подключен
    std::atomic<bool> _networckConnect {false};
    //потоки принятия/отправки запущены? true - запущены
    std::atomic<bool> _networckThreadsSost {false};

    //#################### СТРУКТУРЫ ДАННЫХ ####################
    std::vector<RecordShort> _listRecord;
    mutable std::mutex _listRecord_mutex;

    RecordFull _record_full {};
    mutable std::mutex _record_full_mutex;


    //#################### Уведомления ####################
    mutable std::mutex notifiMutex;
    std::string notifi;

public:
    Mediator(/* args */);
    ~Mediator();

    //#################### Обновление UI ####################

    // регистрация уведомителя (должна вызываться из GUI при инициализации)
    void setUiNotifyCallback(std::function<void()> cb);

    // главный поток вызывает после обработки уведомления
    void clearUiUpdatePending();

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
    //получить флаг наличия НЕ критических ошибок на сервере (true - ошибки)
    bool getSrvStatErr() const;
    //изменить статус флага наличия НЕ критических ошибок на сервере (true - ошибки)
    void setSrvStatErr(bool StatErr);
    //получить флаг наличия ошибок в обмене (true - ошибки)
    bool getSrvStatErrMess() const;
    //изменить статус флага наличия ошибок в обмене (true - ошибки)
    void setSrvStatErrMess(bool StatErrMess);
    
     //      --- Обработка отправки сообщения ---
    //сообщение отправлено в очередь
    bool getSendMessStatus() const;
    //сообщение отправлено в очередь
    void setSendMessStatus(bool sendMessStatus);
    // Сообщение сохранено сервером
    bool getSaveMessStatus() const;
    // Сообщение сохранено сервером
    void setSaveMessStatus(bool statErrMessStatus);


    //#################### СЕТЬ ####################
    
    //получить флаг подкключчения к сети (true - подключены)
    bool getNetworckConnect() const;

    //изменить статус флага подкключчения к сети (true - подключены)
    void setNetworckConnect(bool networckConnect);
    
    //получить флаг запуска потоков сети - true - запущены
    bool getNetworckThreadsSost() const;

    //изменить статус флага запуска потоков сети - true - запущены
    void setNetworckThreadsSost(bool networckThreadsSost);

    //#################### СТРУКТУРЫ ДАННЫХ ####################

    //получить список сохраненных записей 
    std::vector<RecordShort> getListRecord() const;

    //изменить список сохраненных записей 
    void setListRecord(std::vector<RecordShort> && listRecord);


    //получить запись
    RecordFull getRecordFull() const;

    //изменить запись
    void setRecordFull(RecordFull && record_full);


    //#################### Уведомления ####################
    std::string getNotifi() const;
    void setNotifi(std::string notifi);
};


