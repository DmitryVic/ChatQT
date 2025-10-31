#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <memory>
#include <UserStatus.h>

class QTimer;

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
  Q_OBJECT

public:
  explicit LoginForm(QWidget *parent = nullptr, std::shared_ptr<UserStatus> userStatus = nullptr);
  ~LoginForm();

  void setUserStatus(std::shared_ptr<UserStatus> userStatus);

signals:
  void registerRequested();
  void accepted();
  void rejected();

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void on_registrationPushButton_clicked();



private:
  void setControlsEnabled(bool enabled);
  std::shared_ptr<UserStatus> _userStatus;
  Ui::LoginForm *ui;
  QTimer* _authTimer = nullptr;
  int _authAttempts = 0;
};

#endif // LOGINFORM_H
