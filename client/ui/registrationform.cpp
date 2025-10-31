#include "registrationform.h"
#include "./ui_registrationform.h"
#include <QMessageBox>
#include <QTimer>
#include "Logger.h"


RegistrationForm::RegistrationForm(QWidget *parent, std::shared_ptr<UserStatus> userStatus) :
  QWidget(parent), _userStatus(userStatus), ui(new Ui::RegistrationForm),
  _regTimer(new QTimer(this))
{
  ui->setupUi(this);
  _regTimer->setInterval(100); // 100мс между попытками = 20 попыток за 2 секунды
  
  connect(_regTimer, &QTimer::timeout, this, [this]() {
    _regAttempts++;
    if (_regAttempts > 20) // таймаут 20 попыток (~2 секунды)
    {
      _regTimer->stop();
      setControlsEnabled(true);
      ui->serwAnswer->setText("⚠️ Превышено время ожидания ответа от сервера по регистрации");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Timeout waiting for server response"));
      return;
    }
    if(_userStatus->getAuthorizationStatus() && !_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      _regTimer->stop();
      setControlsEnabled(true);
      ui->serwAnswer->setText("✅ Регистрация успешна");
      get_logger() << "✅ Регистрация успешна";
      _userStatus->setServerResponseReg(false);
      emit accepted();
      return;
    }
    else if(_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      _regTimer->stop();
      setControlsEnabled(true);
      ui->serwAnswer->setText("⚠️ Логин занят");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Login is busy"));
      _userStatus->setLoginBusy(false);
      _userStatus->setServerResponseReg(false);
      return;
    }
    else if(!_userStatus->getAuthorizationStatus() && _userStatus->getServerResponseReg()){ //Ошибочные данные, регистрация не прошла
      _regTimer->stop();
      setControlsEnabled(true);
      ui->serwAnswer->setText("⚠️ Ошибка данных для регистрации");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Data error"));
      _userStatus->setServerResponseReg(false);
      return;
    }
  }, Qt::UniqueConnection);
}

void RegistrationForm::setControlsEnabled(bool enabled)
{
  ui->buttonBox->setEnabled(enabled);
  ui->loginEdit->setEnabled(enabled);
  ui->passwordEdit->setEnabled(enabled);
  ui->passwordConfirmEdit->setEnabled(enabled);
  ui->nameEdit->setEnabled(enabled);
  ui->loginButton->setEnabled(enabled);
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
  
  _regAttempts = 0;
  if (_regTimer->isActive()) _regTimer->stop();
  setControlsEnabled(false);
  _regTimer->start();

}



void RegistrationForm::on_buttonBox_rejected()
{
  if (_regTimer && _regTimer->isActive()) _regTimer->stop();
  setControlsEnabled(true);
  emit rejected();
}

void RegistrationForm::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
    _userStatus = userStatus;
}