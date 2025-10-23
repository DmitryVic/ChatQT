#include "registrationform.h"
#include "./ui_registrationform.h"
#include <QMessageBox>

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
  int autotiz = 3;
  switch(autotiz)
  {
  case 1:
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Incorrect login"));
    return;
  case 2:
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Login alredy exists"));
    return;
  default:
    emit accepted();
  }

}

void RegistrationForm::on_buttonBox_rejected()
{
  emit rejected();
}

void RegistrationForm::setUserStatus(std::shared_ptr<UserStatus> userStatus) {
    _userStatus = userStatus;
}