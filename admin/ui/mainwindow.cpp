#include "mainwindow.h"
#include "./ui_mainwindow.h"
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
#include <QScrollBar>
#include <thread>
#include <chrono>
#include "Message.h"
#include "startscreen.h"
#include "Logger.h"

MainWindow::MainWindow (std::shared_ptr<UserStatus> userStatus, QWidget *parent)
: QMainWindow(parent), _userStatus(std::move(userStatus)), ui(new Ui::MainWindow)
{
       ui->setupUi (this);

       setStyleDark();


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

       // запрос на получение спика  юзеров
       Message12 mess12;
       json j12;
       mess12.to_json(j12);
       _userStatus->pushMessageToSend(j12.dump());

       //  запрос на получение списка сообщений
       Message13 mess13;
       json j13;
       mess13.to_json(j13);
       _userStatus->pushMessageToSend(j13.dump());

       // Таймер для отправки запросов на обновление списков
       QTimer* timer_list = new QTimer(this);
       timer_list->setInterval(3000); // обновление каждые 3 секунды
       connect(timer_list, &QTimer::timeout, this, [this]() {
              if (_userStatus->getNetworckConnect())
              {
                     // запрос на получение спика  юзеров
                     Message12 mess12;
                     json j12;
                     mess12.to_json(j12);
                     _userStatus->pushMessageToSend(j12.dump());

                     //  запрос на получение списка сообщений
                     Message13 mess13;
                     json j13;
                     mess13.to_json(j13);
                     _userStatus->pushMessageToSend(j13.dump());
              }
              else{
              // ПРИ ПОТЕРИ СВЯЗИ ПОКА ПРОСТО ЗАКЫВАЕМ, НУЖНО ДОПИСАТЬ ЛОГИКУ ПОВТОРНОГО ЛОГИРОВАНИЯ
              //  ПЕРЕДОВАЯ СООБЩЕНИЕ С ДАННЫМИ getUser И ЛУЧШЕ СДЕЛАТЬ ЭТО В resetMainWind СРАЗУ ПО ИЗМЕНИНЮ СТАТУСА NetworckConnect
                     _userStatus->stopApp();
                     this->close(); // Закрываем окно
              }

       });
       timer_list->start();
}

MainWindow::~MainWindow () { 
       if (_userStatus)
              _userStatus->stopApp();
       delete ui;
 }


MainWindow *MainWindow::createClient(std::shared_ptr<UserStatus> userStatus)
{
       StartScreen s(userStatus, nullptr); //   СОЗДАЕТСЯ StartScreen
       // s.connect_s();
       auto result = s.exec();            //   ПОКАЗЫВАЕТСЯ StartScreen (модально)
       if(result == QDialog::Rejected)
       {
              return nullptr;
       }
       // создается MainWindow после успешного входа (запуск уже в Main)
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
       resetChatListArea();
       resetNotifi();
       if (_userStatus->getResetMess())
       {
              resetMessagesArea();
              _userStatus->setResetMess(false); // сброс флага после обновления
              // get_logger() << "_userStatus->setResetMess(false); // сброс флага после обновления";
       }
}

void MainWindow::resetNotifi(){
       if (_userStatus->getNetworckConnect())
       {
       ui->notifi->setText("🌐 Подключены к серверу");
       }
       else
       {
       ui->notifi->setText("❗ Отсутствует подключение к серверу");
       }
       // ui->notifi->setText(QString::fromStdString(_userStatus->getNotifi()));
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

       std::vector<MessageStructAdmin> message = _userStatus->getMessList();
       
       // Добавляем 
       for (MessageStructAdmin msg : message) {

              QWidget *messageWidget = new QWidget(scrollContentMessages);
              QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);
              messageLayout->setContentsMargins(0, 0, 0, 0);

              // Контент сообщения
              QWidget *contentWidget = new QWidget(messageWidget);
              contentWidget->setObjectName(msg.messFromChatH ? "message-bubble-my" : "message-bubble-other");

              // ВАЖНО: разрешаем рисовать фон у этого виджета
              contentWidget->setAttribute(Qt::WA_StyledBackground, true); // эта строка решает проблему с фоном!


              QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
              contentLayout->setContentsMargins(12, 8, 12, 8); // padding внутри bubble
              contentLayout->setSpacing(4);

              // Текст сообщения
              QLabel *messageText = new QLabel(contentWidget);
              messageText->setObjectName(msg.messFromChatH ? "message-text-my" : "message-text");
              messageText->setWordWrap(true);
              QString typeMess = (msg.messFromChatH) ? "Общий чат:\n" : "Приват\n";
              messageText->setText(QString("%1 %2 :\n %3").arg(typeMess, QString::fromStdString(msg.userName), QString::fromStdString(msg.mess)));

              messageText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
              messageText->setAlignment(Qt::AlignLeft | Qt::AlignTop);

              // Время
              QLabel *timeLabel = new QLabel(contentWidget);
              timeLabel->setObjectName(msg.messFromChatH ? "message-time-my" : "message-time");
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
              if (msg.messFromChatH) {
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

// Полное обновление списка клиентов
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


       //ДОБАВЛЯЕМ ПОЛЬЗОВАТЕЛЕЙ
       std::vector<AdminDataUsers> listUsers = _userStatus->getListUsers(); // Загружаем список пользователей из UserStatus pair<us.login, us.name>
       for (const auto& user : listUsers) {
              
              std::string banB = (user.banStatus) ? " ‼️БАН " : "";
              std::string disB = (user.onlineStatus) ? "" : " ❕ ";
              const std::string& bottonTitle = banB + disB +  user.userName + " (" + user.userLogin + ")";

              QPushButton *chatButton = new QPushButton(scrollContent);
              chatButton->setText(QString::fromStdString(bottonTitle));
              chatButton->setMinimumHeight(50);
              chatButton->setMaximumHeight(50);
              chatButton->setObjectName("chat-button");

              connect(chatButton, &QPushButton::clicked, this, [user, this]() {
              
              SelectedUser sUser;
              sUser.userLogin = user.userLogin;
              sUser.userName = user.userName;
              sUser.ban = user.banStatus;
              sUser.disconn = user.onlineStatus;
              //Добавляем пользователя в выбор действий
              _userStatus->setSelectedUser(sUser);

              // Получаем модель, если есть — очищаем, иначе создаём новую
              QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->selectedUser->model());
              if (!model) {
                     model = new QStandardItemModel(this);
                     ui->selectedUser->setModel(model);
              } else {
                     model->clear(); // очищаем старые элементы
              }
              std::string ban = (sUser.ban) ? " 🔴 " : " 🟢 ";
              std::string dis = (sUser.disconn) ? " 🟢 " : " ⚪ ";
              std::string title = "Выбран пользователь " + sUser.userName + " (" + sUser.userLogin + ") " + 
              "\nСтатус бана " + ban + " отсоединение отправлено: " + dis;

              // Добавляем новый текст (используем переменную chatName, а не повторный вызов)
              QStandardItem *item = new QStandardItem(QString::fromStdString(title));
              item->setEditable(false); // обычно заголовки не редактируемы
              model->appendRow(item);

              });

              scrollLayout->addWidget(chatButton);
       }

       // Добавляем растягивающий элемент, чтобы кнопки прижимались вверх
       scrollLayout->addStretch();

       // // скроллим вниз область сообщений
       // QScrollBar *vScrollBar = ui->scrollAreaMessage->verticalScrollBar();
       // vScrollBar->setValue(vScrollBar->maximum());
       
}



// TO DO переименовать 
// Обновление сообщений тяжелое, пока по кнопке
// TO DO обмен с сервером bool есть ли обновления
// Запрос к БД N ID последней записи, если изменился,
// то послать новую пачку дваннных
void MainWindow::on_pushButtonDMess_clicked()
{
       // скроллим вниз область сообщений
       QScrollBar *vScrollBar = ui->scrollAreaMessage->verticalScrollBar();
       vScrollBar->setValue(vScrollBar->maximum());
}

void MainWindow::on_pushButton_BAN_clicked()
{
       Message11 m11;
       SelectedUser user = _userStatus->getSelectedUser();
       m11.user_login = user.userLogin;
       if (user.userLogin != ""){
              if (user.ban)
                     m11.ban_value = false;
              else
                     m11.ban_value = true;
              
              json j11;
              m11.to_json(j11);
              _userStatus->pushMessageToSend(j11.dump());
       }
        // Получаем модель, если есть — очищаем, иначе создаём новую
       QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->selectedUser->model());
       if (!model) {
              model = new QStandardItemModel(this);
              ui->selectedUser->setModel(model);
       } else {
              model->clear(); // очищаем старые элементы
       }
}


void MainWindow::on_pushButton_DISCONNECT_clicked()
{
       Message10 m10;
       SelectedUser user = _userStatus->getSelectedUser();
       m10.user_login = user.userLogin;
       if (user.userLogin != ""){
              json j10;
              m10.to_json(j10);
              _userStatus->pushMessageToSend(j10.dump());
       }
        // Получаем модель, если есть — очищаем, иначе создаём новую
       QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->selectedUser->model());
       if (!model) {
              model = new QStandardItemModel(this);
              ui->selectedUser->setModel(model);
       } else {
              model->clear(); // очищаем старые элементы
       }
}

