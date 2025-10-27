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
  explicit StartScreen(std::shared_ptr<UserStatus> userStatus, QWidget *parent = nullptr);
  ~StartScreen();

  int userId() const;
  QString userName() const;

  // void connect_s();


public slots:


private:
  std::shared_ptr<UserStatus> _userStatus;
  Ui::StartScreen *ui;
};

#endif // STARTSCREEN_H
