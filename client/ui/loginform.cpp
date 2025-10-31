#include "loginform.h"
#include "./ui_loginform.h"
#include <QMessageBox>
#include <QTimer>
#include "Message.h"
#include "Logger.h"


LoginForm::LoginForm(QWidget *parent, std::shared_ptr<UserStatus> userStatus) :
  QWidget(parent), _userStatus(userStatus), ui(new Ui::LoginForm),
  _authTimer(new QTimer(this))
{
  ui->setupUi(this);
  _authTimer->setInterval(100); // 100мс между попытками = 20 попыток за 2 секунды
  
  connect(_authTimer, &QTimer::timeout, this, [this]() {
    _authAttempts++;
    if (_authAttempts > 20) // таймаут 20 попыток (~2 секунды)
    {
      _authTimer->stop();
      setControlsEnabled(true);
      ui->serverAnswer->setText("⚠️ Превышено время ожидания ответа от сервера по авторизации");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Timeout waiting for server response"));
      return;
    }
    if(_userStatus->getAuthorizationStatus() && !_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      _authTimer->stop();
      setControlsEnabled(true);
      ui->serverAnswer->setText("✅ Авторизация успешна");
      get_logger() << "_userStatus->getAuthorizationStatus(): " << _userStatus->getAuthorizationStatus();
      _userStatus->setServerResponseReg(false);
      emit accepted();
      return;
    }
    else if(_userStatus->getLoginBusy() && _userStatus->getServerResponseReg())
    {
      _authTimer->stop();
      setControlsEnabled(true);
      ui->serverAnswer->setText("⚠️ Логин занят");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Login is busy"));
      _userStatus->setLoginBusy(false);
      _userStatus->setServerResponseReg(false);
      return;
    }
    else if(_userStatus->getServerResponseReg() && !_userStatus->getAuthorizationStatus())
    {
      _authTimer->stop();
      setControlsEnabled(true);
      ui->serverAnswer->setText("⚠️ Неверный логин или пароль");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("Invalid login or password"));
      _userStatus->setServerResponseReg(false);
      return;
    }
  }, Qt::UniqueConnection);
}

void LoginForm::setControlsEnabled(bool enabled)
{
  ui->buttonBox->setEnabled(enabled);
  ui->loginEdit->setEnabled(enabled);
  ui->passwordEdit->setEnabled(enabled);
  ui->registrationPushButton->setEnabled(enabled);
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
  try {
      mess1.to_json(j1);
      std::string jsonStr = j1.dump();
      get_logger() << "Sending JSON: " << jsonStr;
      if (_userStatus == nullptr)
      {
        get_logger() << "Error: _userStatus is null\n";
        return;
      }
      
      _userStatus->pushMessageToSend(jsonStr);
  }
  catch(const std::exception& e) {
      get_logger() << "Error creating/sending message: " << e.what();
      return;
  }
  ui->serverAnswer->setText("🕐 Ожидаем ответ сервера... (2 сек)");
  
  _authAttempts = 0;
  if (_authTimer->isActive()) _authTimer->stop();
  setControlsEnabled(false);
  _authTimer->start();
}

void LoginForm::on_buttonBox_rejected()
{
  if (_authTimer && _authTimer->isActive()) _authTimer->stop();
  setControlsEnabled(true);
  emit rejected();
}

void LoginForm::on_registrationPushButton_clicked()
{
  emit registerRequested();
}


void LoginForm::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
    _userStatus = userStatus;
}