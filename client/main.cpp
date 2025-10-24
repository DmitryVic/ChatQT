
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

  // Универсальная настройка локали
    setlocale(LC_ALL, "ru_RU.UTF-8");

    // Для Windows
    #ifdef _WIN32
      SetConsoleCP(CP_UTF8);
      SetConsoleOutputCP(CP_UTF8);
    #endif

    // Для Linux
    #ifdef SET_GLOBAL_LOCALE_LINUX
    try {
      std::locale::global(std::locale("ru_RU.UTF-8"));
    } catch (const std::exception& e) {
      std::cerr << "Locale error: " << e.what() << std::endl;
      std::locale::global(std::locale("C.UTF-8")); // Fallback
    }
    #endif
    
    std::shared_ptr userStatus = std::make_shared<UserStatus>();
    std::shared_ptr<NetworkClient> network = std::make_shared<NetworkClient>("127.0.0.1", PORT, userStatus);

     // Создаем обработчики
    auto Handler50 = std::make_shared<HandlerMessage50>(userStatus);
    auto Handler51 = std::make_shared<HandlerMessage51>(userStatus);
    auto Handler52 = std::make_shared<HandlerMessage52>(userStatus);
    auto Handler53 = std::make_shared<HandlerMessage53>(userStatus);
    auto Handler54 = std::make_shared<HandlerMessage54>(userStatus);
    auto Handler55 = std::make_shared<HandlerMessage55>(userStatus);
    auto Handler56 = std::make_shared<HandlerMessage56>(userStatus);
    auto messageError = std::make_shared<HandlerErr>(userStatus);
    // Связываем
    messageError->setNext(nullptr);
    Handler56->setNext(messageError);
    Handler55->setNext(Handler56);
    Handler54->setNext(Handler55);
    Handler53->setNext(Handler54);
    Handler52->setNext(Handler53);
    Handler51->setNext(Handler52);
    Handler50->setNext(Handler51);

    thread core([userStatus, Handler50, network](){
        
      while (userStatus->running())
      {
          while (userStatus->getNetworckConnect())
          {
              std::string json_str = userStatus->waitAndPopAcceptedMessage();
              
              if(json_str == "") {
                  cout << "Пусто\n";
                  break;
              }

              auto msg = parse_message(json_str);
                          
              if (!msg)
              {
                  userStatus->setNetworckConnect(false);
                  // throw  std::runtime_error("Ошибка в полученых данных");
                  std::cerr << "core | Ошибка в полученых данных\n";
              }
              
              if (!Handler50->handle(msg)){
                  // userStatus->setNetworckConnect(false);
                  std::cerr << "core | Ошибка в обработке данных\n";
                  // throw  std::runtime_error("Ошибка в обработке данных, закрываю соединение...");
              }
          }
          
          std::cerr << "Запускаю инициализацию сети\n";
          userStatus->setNotifi("Запускаю инициализацию сети");
          
          if (userStatus->running())
          {
              std::cerr << "Остановка Потоков\n";    
              if (userStatus->getNetworckThreadsSost())
                  network->stopThreads();

              for (size_t i = 1; i < 4; i++)
              {
                  std::this_thread::sleep_for(std::chrono::seconds(1));
                  std::string notify = "Network reconnect: " + std::to_string(i) + " sec";
                  userStatus->setNotifi(notify);
                  std::cerr << "Network reconnect: "  << i  << " sec\n";
              }

              if (userStatus->getNetworckThreadsSost()){
                  std::cerr << "Повторно останавливаю потоки сети\n";
                  userStatus->setNotifi("Повторно останавливаю потоки сети");
              }
              else
              {
                  std::cerr << "Подключение сети\n"; 
                  userStatus->setNotifi("Подключение сети");
                  network->connecting();
                  std::this_thread::sleep_for(std::chrono::seconds(1));
                  std::cerr << "Запуск потоков\n";
                  userStatus->setNotifi("Запуск потоков");    
                  network->startThreads();
              }
          }
          else
          {
              std::cerr << "Остановка Потоков userStatus->running() OFF\n";    
              network->stopThreads();
          }   
      }
    });

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
  auto w = MainWindow::createClient(userStatus);
  //          |
  //          └── внутри: StartScreen s; s.exec(); ← ОКНО ВХОДА ПОКАЗЫВАЕТСЯ ЗДЕСЬ
  //              s.exec() блокирует дальнейшее выполнение до закрытия StartScreen

  if(w)
    w->show();          //   ГЛАВНОЕ ОКНО ПОКАЗЫВАЕТСЯ ЗДЕСЬ!
  else
    {
      // Пользователь не вошел - завершаем приложение
      std::cerr << "Пользователь не вошел - завершаем приложение\n";
      if (network) network->stopThreads();
      if (userStatus) userStatus->stopApp();

      if (core.joinable()) core.join();

      return 0;
    }

  //Запуск UI
  int exitCode = a.exec();
  
  // Остановка приложения
  if (network) network->stopThreads();
  if (userStatus) userStatus->stopApp();

  if (core.joinable()) core.join();

  return exitCode;
}
