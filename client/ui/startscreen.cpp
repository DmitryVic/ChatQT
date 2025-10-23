#include "startscreen.h"
#include "./ui_startscreen.h"

StartScreen::StartScreen(QWidget *parent, std::shared_ptr<UserStatus> userStatus) :
  QDialog(parent), _userStatus(userStatus), ui(new Ui::StartScreen)
{
  ui->setupUi(this);
  
  // Передаём userStatus во вложенные формы
  ui->loginWidget->setUserStatus(_userStatus);
  ui->registerWidget->setUserStatus(_userStatus);
  
  connect(ui->loginWidget, &LoginForm::registerRequested, this, &StartScreen::setRegistrationForm);
  connect(ui->loginWidget, &LoginForm::accepted, this, &StartScreen::onLoggedIn);
  connect(ui->loginWidget, &LoginForm::rejected, this, &StartScreen::onRejectRequested);
  connect(ui->registerWidget, &RegistrationForm::loginRequested, this, &StartScreen::setLoginForm);
  connect(ui->registerWidget, &RegistrationForm::accepted, this, &StartScreen::onLoggedIn);
  connect(ui->registerWidget, &RegistrationForm::rejected, this, &StartScreen::onRejectRequested);

}

StartScreen::~StartScreen()
{
  delete ui;
}

void StartScreen::setLoginForm()
{
  ui->stackedWidget->setCurrentIndex(0);
}

void StartScreen::setRegistrationForm()
{
  ui->stackedWidget->setCurrentIndex(1);
}

void StartScreen::onLoggedIn()
{
  accept();
}

void StartScreen::onRejectRequested()
{
  reject();
}

void StartScreen::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
  _userStatus = userStatus;
  // Пробросить указатель в дочерние виджеты, если UI уже инициализирован
  if (ui) {
    if (ui->loginWidget) ui->loginWidget->setUserStatus(_userStatus);
    if (ui->registerWidget) ui->registerWidget->setUserStatus(_userStatus);
  }
}