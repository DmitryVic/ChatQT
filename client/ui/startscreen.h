#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <memory>
#include <UserStatus.h>

namespace Ui {
class StartScreen;
}

class StartScreen : public QDialog
{
  Q_OBJECT

public:
  explicit StartScreen(QWidget *parent = nullptr, std::shared_ptr<UserStatus> userStatus = nullptr);
  ~StartScreen();
  void setLoginForm();
  void setRegistrationForm();

  int userId() const;
  QString userName() const;

  void setUserStatus(std::shared_ptr<UserStatus> userStatus);


public slots:
  void onLoggedIn();
  void onRejectRequested();

private:
  std::shared_ptr<UserStatus> _userStatus;
  Ui::StartScreen *ui;
};

#endif // STARTSCREEN_H
