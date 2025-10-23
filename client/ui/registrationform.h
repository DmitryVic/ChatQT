#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>
#include <memory>
#include <UserStatus.h>

namespace Ui {
class RegistrationForm;
}

class RegistrationForm : public QWidget
{
  Q_OBJECT

public:
  explicit RegistrationForm(QWidget *parent = nullptr, std::shared_ptr<UserStatus> userStatus = nullptr);
  ~RegistrationForm();

  void setUserStatus(std::shared_ptr<UserStatus> userStatus);

signals:
  void loginRequested();
  void accepted();
  void rejected();

private slots:
  void on_loginButton_clicked();
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

private:
  std::shared_ptr<class UserStatus> _userStatus;
  Ui::RegistrationForm *ui;
};

#endif // REGISTRATIONFORM_H
