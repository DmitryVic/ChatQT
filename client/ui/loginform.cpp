#include "loginform.h"
#include "./ui_loginform.h"
#include <QMessageBox>
#include "Message.h"
#include <thread>
#include <chrono>


LoginForm::LoginForm(QWidget *parent, std::shared_ptr<UserStatus> userStatus) :
  QWidget(parent), _userStatus(userStatus), ui(new Ui::LoginForm)
{
  ui->setupUi(this);
}

LoginForm::~LoginForm()
{
  delete ui;
}

void LoginForm::on_buttonBox_accepted()
{
  bool autotiz = true;

  if(!autotiz)
  {
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Password is wrong"));
    return;
  }

  Message1 mess1;
  mess1.login = ui->loginEdit->text().toStdString();
  mess1.pass = ui->passwordEdit->text().toStdString();
  json j1;
  // mess1.to_json(j1);
  // _userStatus->pushMessageToSend(j1.dump());
  try {
      mess1.to_json(j1);
      std::string jsonStr = j1.dump();
      std::cerr << "Sending JSON: " << jsonStr << std::endl;
      if (_userStatus == nullptr)
      {
        std::cerr << "Error: _userStatus is null\n";
        return;
      }
      
      _userStatus->pushMessageToSend(jsonStr);
  }
  catch(const std::exception& e) {
      std::cerr << "Error creating/sending message: " << e.what() << std::endl;
      return;
  }
  std::cerr << "Ожидаем ответ сервера по регистрации...\n";
  
  int attempts = 0;
  while (true)
  {
    attempts++;
    if (attempts > 20) // таймаут 20 попыток (~2 секунды)
    {
      std::cerr << "Превышено время ожидания ответа от сервера по регистрации.\n";
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Timeout waiting for server response"));
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if(_userStatus->getAuthorizationStatus() && !_userStatus->getLoginBusy())
    {
      std::cerr << "Регистрация успешна.\n";
       // уведомляем о успешной регистрации
       emit accepted();
      return;
    }
    else if(_userStatus->getLoginBusy())
    {
      std::cerr << "Логин занят, регистрация не удалась.\n";
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Login is busy"));
      _userStatus->setLoginBusy(false); // сбрасываем флаг
      return;
    }
  }
 
}

void LoginForm::on_buttonBox_rejected()
{
  emit rejected();
}

void LoginForm::on_registrationPushButton_clicked()
{
  emit registerRequested();
}


void LoginForm::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
    _userStatus = userStatus;
}