#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "startscreen.h"
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDebug>
#include <memory>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDateTime>
#include "UserStatus.h"
#include <QPointer>
#include <QTimer>

MainWindow::MainWindow (std::shared_ptr<UserStatus> userStatus, QWidget *parent)
: QMainWindow(parent), _userStatus(std::move(userStatus)), ui(new Ui::MainWindow)
{
       ui->setupUi (this);

       setStyleDark();


       ui->messButtonPush->setText("Отправить 📨");

       // Таймер для проверки обновлений из рабочих потоков
       QTimer* timer = new QTimer(this);
       timer->setInterval(200);
       connect(timer, &QTimer::timeout, this, [this]() {
              if (_userStatus->hasUpdatePending()) {
              _userStatus->clearUpdatePending();
              resetMainWind(); // Вызывается в основном потоке Qt
              }
       });
       timer->start();

       /////////////////////////////// Обновления ///////////////////////////////

       // Отправка запроса о получении списка приватных чатов
       Message5 mess5;
       mess5.my_login = _userStatus->getUser().getLogin();
       json j5;
       mess5.to_json(j5);
       _userStatus->pushMessageToSend(j5.dump());

       // Отправка запроса о получении списока всех юзеров в чате кому написать
       Message5 mess6;
       mess6.my_login = _userStatus->getUser().getLogin();
       json j6;
       mess6.to_json(j6);
       _userStatus->pushMessageToSend(j6.dump());

       // Таймер для отправки запросов на обновление списков
       QTimer* timer_list = new QTimer(this);
       timer_list->setInterval(2000);
       connect(timer_list, &QTimer::timeout, this, [this]() {
              // Отправка запроса о получении списка приватных чатов
              Message5 mess5;
              mess5.my_login = _userStatus->getUser().getLogin();
              json j5;
              mess5.to_json(j5);
              _userStatus->pushMessageToSend(j5.dump());

              // Отправка запроса о получении списока всех юзеров в чате кому написать
              Message5 mess6;
              mess6.my_login = _userStatus->getUser().getLogin();
              json j6;
              mess6.to_json(j6);
              _userStatus->pushMessageToSend(j6.dump());
       });
       timer_list->start();
}

MainWindow::~MainWindow () { delete ui; }


MainWindow *MainWindow::createClient(std::shared_ptr<UserStatus> userStatus)
{
       StartScreen s(nullptr, userStatus); //   СОЗДАЕТСЯ StartScreen
       auto result = s.exec();            //   ПОКАЗЫВАЕТСЯ StartScreen (модально)
       if(result == QDialog::Rejected)
       {
       return nullptr;
       }
       // Дальше создается MainWindow после успешного входа (запуск уже в Main)
       auto w = new MainWindow(userStatus);
       // w->setUserStatus(userStatus);
       w->setAttribute(Qt::WA_DeleteOnClose); //Удалит если закроем!
       QPointer<MainWindow> safeThis = w;

       return w;
}

void MainWindow::on_styleButton_clicked()
{
  static bool darkStyle = true;
  if(darkStyle){
    setStyleLight();
    darkStyle = false;
  } else{
      setStyleDark();
      darkStyle = true;
  }
  resetMainWind();
}


void MainWindow::setUserStatus(std::shared_ptr<UserStatus> userStatus){
       this->_userStatus = std::move(userStatus);
}


 // обновление от UserStatus
void MainWindow::resetMainWind(){
       resetMessagesArea();
       resetChatListArea();
}


void MainWindow::clearMessagesArea() {
    QLayout *layout = ui->scrollAreaWidgetContents->layout();
    if (!layout) return;

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void MainWindow::resetMessagesArea() {
       if (!_userStatus) return;

       clearMessagesArea();

       // Настройка правой панели с сообщениями
       QWidget *scrollContentMessages = ui->scrollAreaWidgetContents;

       // Попробуем получить уже существующий QVBoxLayout проверка — есть ли уже layout
       QVBoxLayout *scrollLayoutMessages = qobject_cast<QVBoxLayout*>(scrollContentMessages->layout());
       if (!scrollLayoutMessages) {
              // Если layout ещё нет — создаём и устанавливаем его один раз
              scrollLayoutMessages = new QVBoxLayout(scrollContentMessages);
              scrollLayoutMessages->setAlignment(Qt::AlignTop);
              scrollLayoutMessages->setSpacing(5);
              scrollLayoutMessages->setContentsMargins(8, 8, 8, 8);
              scrollContentMessages->setLayout(scrollLayoutMessages);
       } else {
              // Если layout уже есть то сбросить его настройки
              scrollLayoutMessages->setAlignment(Qt::AlignTop);
              scrollLayoutMessages->setSpacing(5);
              scrollLayoutMessages->setContentsMargins(8, 8, 8, 8);
       }

       std::vector<MessageStruct> message = _userStatus->getMessList();
       
       // Добавляем 
       for (MessageStruct msg : message) {
              bool isMyMessage = msg.userLogin == _userStatus->getUser().getLogin();

              QWidget *messageWidget = new QWidget(scrollContentMessages);
              QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);
              messageLayout->setContentsMargins(0, 0, 0, 0);

              // Контент сообщения
              QWidget *contentWidget = new QWidget(messageWidget);
              contentWidget->setObjectName(isMyMessage ? "message-bubble-my" : "message-bubble-other");

              // ВАЖНО: разрешаем рисовать фон у этого виджета
              contentWidget->setAttribute(Qt::WA_StyledBackground, true); // эта строка решает проблему с фоном!


              QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
              contentLayout->setContentsMargins(12, 8, 12, 8); // padding внутри bubble
              contentLayout->setSpacing(4);

              // Текст сообщения
              QLabel *messageText = new QLabel(contentWidget);
              messageText->setObjectName(isMyMessage ? "message-text-my" : "message-text");
              messageText->setWordWrap(true);

              messageText->setText(QString("%1: %2").arg(QString::fromStdString(msg.userName), QString::fromStdString(msg.mess)));

              messageText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
              messageText->setAlignment(Qt::AlignLeft | Qt::AlignTop);

              // Время
              QLabel *timeLabel = new QLabel(contentWidget);
              timeLabel->setObjectName(isMyMessage ? "message-time-my" : "message-time");
              timeLabel->setText(QString::fromStdString(timestampToString(msg.time)));
              timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

              // Собираем
              contentLayout->addWidget(messageText);
              QHBoxLayout *metaLayout = new QHBoxLayout();
              metaLayout->addStretch();
              metaLayout->addWidget(timeLabel);
              contentLayout->addLayout(metaLayout);
              contentWidget->setLayout(contentLayout);

              // Ограничиваем ширину bubble
              contentWidget->setMaximumWidth(400);

              // Выравнивание bubble по правому/левому краю
              if (isMyMessage) {
              messageLayout->addStretch();
              messageLayout->addWidget(contentWidget);
              } else {
              messageLayout->addWidget(contentWidget);
              messageLayout->addStretch();
              }

              messageWidget->setLayout(messageLayout);
              scrollLayoutMessages->addWidget(messageWidget);
       }

              // Добавляем растягивающийся элемент в конец
       scrollLayoutMessages->addStretch();

       std::string chatName = _userStatus->getChatName();

       // Получаем модель, если есть — очищаем, иначе создаём новую
       QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->chatName->model());
       if (!model) {
              model = new QStandardItemModel(this);
              ui->chatName->setModel(model);
       } else {
              model->clear(); // очищаем старые элементы
       }

       // Добавляем новый текст (используем переменную chatName, а не повторный вызов)
       QStandardItem *item = new QStandardItem(QString::fromStdString(chatName));
       item->setEditable(false); // обычно заголовки не редактируемы
       model->appendRow(item);
}


// Очистка области списка чатов
void MainWindow::clearChatListArea()
{
    // Получаем layout из scrollAreaWidgetContentsListChat
    QLayout *layout = ui->scrollAreaWidgetContentsListChat->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget(); // удаляем виджет (кнопку)
            }
            delete item; // и сам item
        }
    }
}

// Полное обновление списка чатов
void MainWindow::resetChatListArea()
{
       // Сначала очищаем старые кнопки
       clearChatListArea();

       // Получаем или создаём layout
       QWidget *scrollContent = ui->scrollAreaWidgetContentsListChat;

       // проверка, есть ли layout
       QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout *>(scrollContent->layout());
       if (!scrollLayout) {
              scrollLayout = new QVBoxLayout(scrollContent);
              scrollLayout->setAlignment(Qt::AlignTop);
              scrollLayout->setSpacing(4);
              scrollContent->setLayout(scrollLayout);
       } else { // обновляем настройки существующего layout
              scrollLayout->setAlignment(Qt::AlignTop);
              scrollLayout->setSpacing(4);
       }

       //ДОБАВЛЯЕМ ОБЩИЙ ЧАТ
       QPushButton *chatButton = new QPushButton(scrollContent);
       chatButton->setText(QString::fromStdString("Общий чат"));
       chatButton->setMinimumHeight(50);
       chatButton->setMaximumHeight(50);
       chatButton->setObjectName("chat-button");

       connect(chatButton, &QPushButton::clicked, this, [this]() {
              // запрос на получение данных общео чата
              Message9 mess9;
              mess9.user_sender = _userStatus->getUser().getLogin();
              json j9;
              mess9.to_json(j9);
              _userStatus->pushMessageToSend(j9.dump());
       });

       scrollLayout->addWidget(chatButton);

       //ДОБАВЛЯЕМ ПРИВАТНЫЙЕ ЧАТЫ
       std::vector<std::pair<std::string, std::string>> lastChatP = _userStatus->getListChatP(); // Загружаем список чатов из UserStatus pair<us.login, us.name>

       for (const auto& chat : lastChatP) {
              const std::string& chatLogin = chat.first;
              const std::string& chatName = chat.second;

              QPushButton *chatButton = new QPushButton(scrollContent);
              chatButton->setText(QString::fromStdString(chatName));
              chatButton->setMinimumHeight(50);
              chatButton->setMaximumHeight(50);
              chatButton->setObjectName("chat-button");

              connect(chatButton, &QPushButton::clicked, this, [chatLogin, this]() {
              // запрос на получение данных приватного чата
              Message8 mess8;
              mess8.user_sender = _userStatus->getUser().getLogin();
              mess8.user_recipient = chatLogin;
              json j8;
              mess8.to_json(j8);
              _userStatus->pushMessageToSend(j8.dump());
              });

              scrollLayout->addWidget(chatButton);
       }

       //ДОБАВЛЯЕМ ПОЛЬЗОВАТЕЛЕЙ
        std::vector<std::pair<std::string, std::string>> listUsers = _userStatus->getListUsers(); // Загружаем список пользователей из UserStatus pair<us.login, us.name>
       for (const auto& user : listUsers) {
              const std::string& userLogin = user.first;
              const std::string& userName = user.second;

              QPushButton *chatButton = new QPushButton(scrollContent);
              chatButton->setText(QString::fromStdString(userName));
              chatButton->setMinimumHeight(50);
              chatButton->setMaximumHeight(50);
              chatButton->setObjectName("chat-button");

              connect(chatButton, &QPushButton::clicked, this, [userLogin, this]() {
              qDebug() << "Выбран пользователь:" << QString::fromStdString(userLogin);
              ////////////////////////////////////////////////////////////////////////////////
              ////////// TO DO
              ////////////////////////////////////////////////////////////////////////////////
              // запрос на получение данных приватного чата
              Message8 mess8;
              mess8.user_sender = _userStatus->getUser().getLogin();
              mess8.user_recipient = userLogin;
              json j8;
              mess8.to_json(j8);
              _userStatus->pushMessageToSend(j8.dump());
              });

              scrollLayout->addWidget(chatButton);
       }

       // Добавляем растягивающий элемент, чтобы кнопки прижимались вверх
       scrollLayout->addStretch();
}