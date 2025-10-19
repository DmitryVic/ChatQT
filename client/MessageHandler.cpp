#include "MessageHandler.h"
#include "Message.h"
#include "Mediator.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <memory>
#include "MessageHandler.h"


bool MessageHandler::handleNext(const std::shared_ptr<Message>& message) {
    if (_next) return _next->handle(message);
    return false;
}

// Обработка для Message101
// Сообщение о наличии (true) или отсутствие (false) критических ошибок сервера
bool HandlerMessage101::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 101) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем
    try
    {
        std::shared_ptr<Message101> mess = std::dynamic_pointer_cast<Message101>(message);
        this->_Mediator->setSrvStatErrFatall(mess->status_answer_FatallErr);
        this->_Mediator->setSrvStatErr(mess->status_answer_Err);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}


// Обработка для Message102
// Получена запись
bool HandlerMessage102::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 102) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем

    try
    {
        auto m102 = std::dynamic_pointer_cast<Message102>(message);
        RecordFull record_full = m102->record_full;
        this->_Mediator->setRecordFull(std::move(record_full)); //Перемещение!
        
    }
    catch(const std::exception& e)
    {
        std::cerr << "Ошибка обработки HandlerMessage102: " << e.what() << '\n';
        return false;
    }

    return true;
}

// Обработка для Message103
// Запись отправлена
bool HandlerMessage103::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 103) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем

    try
    {
        auto m103 = std::dynamic_pointer_cast<Message103>(message);
        if (m103->save)
        {
            _Mediator->setSaveMessStatus(true);
        }
        else
        {
            _Mediator->setSaveMessStatus(false);
        }
        
        
    }
    catch(const std::exception& e)
    {
        std::cerr << "Ошибка обработки HandlerMessage102: " << e.what() << '\n';
        return false;
    }

    return true;
}

// Обработка для Message104
// Запись отправлена
bool HandlerMessage104::handle(const std::shared_ptr<Message>& message) {
    // Проверяем, наше ли это сообщение
    if (message->getTupe() != 104) {
        // Не наше - передаем следующему в цепочке
        return handleNext(message);
    }
    //обрабатываем

    try
    {
        auto m104 = std::dynamic_pointer_cast<Message104>(message);
        std::vector<RecordShort> l_record;
        for (auto mesRecord : m104->list_record)
        {
            RecordShort tempRecord;
            tempRecord.id = mesRecord.id;
            tempRecord.mode = mesRecord.mode;
            tempRecord.time = mesRecord.time;
            tempRecord.title = mesRecord.title;
            l_record.push_back(tempRecord);
        }

        this->_Mediator->setListRecord(std::move(l_record)); //Перемещение!
        
    }
    catch(const std::exception& e)
    {
        std::cerr << "Ошибка обработки HandlerMessage102: " << e.what() << '\n';
        return false;
    }

    return true;
}



// Обработка неизвестного сообщения
bool HandlerErr::handle(const std::shared_ptr<Message>& message) {
    //обрабатываем
    this->_Mediator->setSrvStatErrMess(true);
    std::cerr << "Ошибка обработки HandlerErr: не верный тип сиибщения" << '\n';
    return true;
}