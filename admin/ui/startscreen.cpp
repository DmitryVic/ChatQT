#include "startscreen.h"
#include "./ui_startscreen.h"
#include <QTimer>
#include "Logger.h"
#include <thread>
#include <chrono>

StartScreen::StartScreen(std::shared_ptr<UserStatus> userStatus, QWidget *parent) :
  QDialog(parent), _userStatus(userStatus), ui(new Ui::StartScreen)
{
  ui->setupUi(this);
  
  // Таймер отображения статуса сети
  QTimer* timer = new QTimer(this);
  timer->setInterval(200);
  connect(timer, &QTimer::timeout, this, [this, userStatus]() {
  if (userStatus->getNetworckConnect())
  {
    ui->networckLabel->setText("✅ Подключены к серверу");
  }
  else
  {
    ui->networckLabel->setText("❗ Отсутствует подключение к серверу");
  }
  });
  timer->start();
  connect_s();
}

StartScreen::~StartScreen()
{
  delete ui;
}

// reject();
// accept();

void StartScreen::connect_s(){

  Message1 mess1;
  mess1.login = _userStatus->getUser().getLogin();
  mess1.pass = _userStatus->getUser().getPass();
  json j1;
  // mess1.to_json(j1);
  // _userStatus->pushMessageToSend(j1.dump());
  try {
      mess1.to_json(j1);
      std::string jsonStr = j1.dump();
      if (_userStatus == nullptr)
      {
        get_logger() << "Error: _userStatus is null\n";
        reject();
        return;
      }
      
      _userStatus->pushMessageToSend(jsonStr);
  }
  catch(const std::exception& e) {
      get_logger() << "Error creating/sending message: " << e.what();
      reject();
      return;
  }
  ui->labelInfo->setText("🕐 Ожидаем ответ сервера... (2 сек)");
  
  int attempts = 0;
  while (true)
  {
    attempts++;
    if (attempts > 20) // таймаут 20 попыток (~2 секунды)
    {
      ui->labelInfo->setText("⚠️ Превышено время ожидания ответа от сервера по авторизации ");
      reject();
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if(_userStatus->getAuthorizationStatus()  && _userStatus->getServerResponseReg())
    {
      ui->labelInfo->setText("✅ Авторизация успешна");
      get_logger() << "_userStatus->getAuthorizationStatus(): " << _userStatus->getAuthorizationStatus();
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
       // уведомляем о успешной регистрации
       accept();
      return;
    }
    else if(_userStatus->getServerResponseReg() && !_userStatus->getAuthorizationStatus()) // пришел ответ с false
    {
      ui->labelInfo->setText("⚠️ Не верный логин или пароль");
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
      reject();
      return;
    }
    
  }

}
