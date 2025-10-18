#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int
main (int argc, char *argv[])
{
  QApplication a (argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system ().uiLanguages ();
  for (const QString &locale : uiLanguages)
    {
      const QString baseName = "Chat_client_" + QLocale (locale).name ();
      if (translator.load (":/i18n/" + baseName))
        {
          a.installTranslator (&translator);
          break;
        }
    }
  // MainWindow w;
  // w.show ();
  // return a.exec ();
  auto w = MainWindow::createClient();
  //          |
  //          └── внутри: StartScreen s; s.exec(); ← ОКНО ВХОДА ПОКАЗЫВАЕТСЯ ЗДЕСЬ
  //              s.exec() блокирует дальнейшее выполнение до закрытия StartScreen

  if(w)
    w->show();          //   ГЛАВНОЕ ОКНО ПОКАЗЫВАЕТСЯ ЗДЕСЬ!
  else
    return 0;           //   цикл событий для главного окна
  return a.exec();
}
