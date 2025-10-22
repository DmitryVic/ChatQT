
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "mainwindow.h"

#include "Message.h"
#include "NetworkClient.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  // pragma comment for linking ws2_32 only for MSVC
  #if defined(_MSC_VER)
  #pragma comment(lib, "ws2_32.lib") // Подключаем библиотеку Winsock
  #endif
  typedef int socklen_t;
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include "UserStatus.h"
#include <nlohmann/json.hpp>
#include "User.h"
#include "MessageHandler.h"
//локаль
#include <locale>
#include <clocale>

#ifdef _WIN32
    #include <windows.h>
#endif


using namespace std;
using json = nlohmann::json;

#define PORT 7777


int main (int argc, char *argv[])
{

  // Для Linux/Unix: отключаем плагин
  // Ошибка: QSocketNotifier: Can only be used with threads started with QThread
  // на Linux GNOME Qt подгружает плагин который при инициализации создает потоки, конфликт Qt QThread
  #ifdef Q_OS_UNIX
      qputenv("QT_IM_MODULE", QByteArray("xim"));
  #endif

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
