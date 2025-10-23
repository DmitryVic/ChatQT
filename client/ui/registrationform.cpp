#include "registrationform.h"
#include "./ui_registrationform.h"
#include <QMessageBox>
#include <thread>
#include <chrono>


RegistrationForm::RegistrationForm(QWidget *parent, std::shared_ptr<UserStatus> userStatus) :
  QWidget(parent), _userStatus(userStatus), ui(new Ui::RegistrationForm)
{
  ui->setupUi(this);
}

RegistrationForm::~RegistrationForm()
{
  delete ui;
}


void RegistrationForm::on_loginButton_clicked()
{
  emit loginRequested();
}


void RegistrationForm::on_buttonBox_accepted()
{
  if(ui->passwordEdit->text() !=
     ui->passwordConfirmEdit->text())
  {
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Passwords not match"));
    return;
  }
 Message2 mess2;
  mess2.login = ui->loginEdit->text().toStdString();
  mess2.pass = ui->passwordEdit->text().toStdString();
  mess2.name = ui->nameEdit->text().toStdString();

  json j2;
  mess2.to_json(j2);
  _userStatus->pushMessageToSend(j2.dump());

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
      emit accepted(); // уведомляем о успешной регистрации
      return;
    }
    else if(_userStatus->getLoginBusy())
    {
      std::cerr << "Логин занят, регистрация не удалась.\n";
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Login is busy"));
      return;
    }
  }
}



void RegistrationForm::on_buttonBox_rejected()
{
  emit rejected();
}

void RegistrationForm::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
    _userStatus = userStatus;
}