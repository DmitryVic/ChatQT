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

  ui->serwAnswer->setText("🕐 Ожидаем ответ сервера ... (2 сек)");
  
  int attempts = 0;
  while (true)
  {
    attempts++;
    if (attempts > 20) // таймаут 20 попыток (~2 секунды)
    {
      ui->serwAnswer->setText("⚠️ Превышено время ожидания ответа от сервера по регистрации ");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Timeout waiting for server response"));
      return;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if(_userStatus->getAuthorizationStatus() && !_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      std::cerr << "✅ Регистрация успешна";
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
      emit accepted(); // уведомляем о успешной регистрации
      return;
    }
    else if(_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      ui->serwAnswer->setText("⚠️ Логин занят");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Login is busy"));
      _userStatus->setLoginBusy(false); // Сбрасываем флаг
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
      return;
    }
    else if(!_userStatus->getAuthorizationStatus() && _userStatus->getServerResponseReg()){ //Ошибочные данные, регистрация не прошла
      ui->serwAnswer->setText("⚠️ Ошибка данных для регистрации");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Data error"));
      _userStatus->setServerResponseReg(false); // сбрасываем флаг
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