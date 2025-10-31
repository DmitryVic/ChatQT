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
  
  ui->labelInfo->setText("🕐 Ожидаем ответ сервера...");


  Message1 mess1;
  mess1.login = userStatus->getUser().getLogin();
  mess1.pass = userStatus->getUser().getPass();
  json j1;


  try {
    mess1.to_json(j1);
    std::string jsonStr = j1.dump();
    if (userStatus == nullptr)
    {
      get_logger() << "Error: userStatus is null\n";
      reject();
      return;
    }
    userStatus->pushMessageToSend(jsonStr);
  }
  catch(const std::exception& e) {
      get_logger() << "Error creating/sending message: " << e.what();
      reject();
      return;
  }


  const int intervalMs = 100;

  QTimer *timerConnect = new QTimer(this);
  timerConnect->setInterval(intervalMs);

  connect(timerConnect, &QTimer::timeout, this, [this, timerConnect]()  {

    get_logger() << "timerConnect userStatus->getAuthorizationStatus() " << _userStatus->getAuthorizationStatus();
    if(_userStatus->getAuthorizationStatus())
    {
      timerConnect->stop(); 
      ui->labelInfo->setText("✅ Авторизация успешна");
      get_logger() << "_userStatus->getAuthorizationStatus(): " << _userStatus->getAuthorizationStatus();
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
      // уведомляем о успешной регистрации
      accept();
      return;
    }
    else if(_userStatus->getServerResponseReg()) // пришел ответ с false
    {
      timerConnect->stop(); 
      ui->labelInfo->setText("⚠️ Не верный логин или пароль");
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
      reject();
      return;
    }
        
  });

  timerConnect->start();
}

StartScreen::~StartScreen()
{
  delete ui;
}
