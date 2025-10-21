#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "UserStatus.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (QWidget *parent = nullptr, std::shared_ptr<class UserStatus> userStatus = nullptr);
  ~MainWindow ();
  
  static MainWindow* createClient();
  void setStyleDark();
  void setStyleLight();
  void setPtrUserStatus(std::shared_ptr<UserStatus> userStatus);
  
  void clearMessagesArea(); // очистка области сообщений
  void resetMessagesArea(); // обновление области сообщений

  void resetChatListArea(); // обновление области списка чатов
  void clearChatListArea(); // очистка области списка чатов

  void resetMainWind();

  private slots:
  void on_styleButton_clicked();

private:
  Ui::MainWindow *ui;
  std::shared_ptr<class UserStatus> _userStatus;
};
#endif // MAINWINDOW_H
