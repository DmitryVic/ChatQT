#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
  MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();
  
  static MainWindow* createClient();
  void setStyleDark();
  void setStyleLight();
private slots:
  void on_styleButton_clicked();

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
