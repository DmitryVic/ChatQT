#include "loginform.h"
#include "./ui_loginform.h"
#include <QMessageBox>

LoginForm::LoginForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::LoginForm)
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

  emit accepted();
}

void LoginForm::on_buttonBox_rejected()
{
  emit rejected();
}

void LoginForm::on_registrationPushButton_clicked()
{
  emit registerRequested();
}


