#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <memory>

namespace Ui {
class StartScreen;
}

class StartScreen : public QDialog
{
  Q_OBJECT

public:
  explicit StartScreen(QWidget *parent = nullptr);
  ~StartScreen();
  void setLoginForm();
  void setRegistrationForm();

  int userId() const;
  QString userName() const;


public slots:
  void onLoggedIn();
  void onRejectRequested();

private:
  Ui::StartScreen *ui;
};

#endif // STARTSCREEN_H
