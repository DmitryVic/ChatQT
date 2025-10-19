#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "startscreen.h"
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDebug>

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDateTime>


MainWindow::MainWindow (QWidget *parent)
    : QMainWindow (parent), ui (new Ui::MainWindow)
{
       ui->setupUi (this);

       setStyleDark();

              // Получаем виджет содержимого scrollArea
       QWidget *scrollContent = ui->scrollAreaWidgetContentsListChat;

       // Создаем вертикальный layout для содержимого
       QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
       scrollLayout->setAlignment(Qt::AlignTop); // Выравнивание по верху
        scrollLayout->setSpacing(4); // Уменьшаем расстояние между кнопками
       scrollContent->setLayout(scrollLayout);

              // Добавляем 50 кнопок-чатов
       for (int i = 1; i <= 50; ++i) {
       QPushButton *chatButton = new QPushButton(scrollContent);
       chatButton->setText(QString("Чат %1").arg(i));
       chatButton->setMinimumHeight(50);
       chatButton->setMaximumHeight(50);
       chatButton->setObjectName("chat-button");

              // Подключаем обработчик нажатия
       connect(chatButton, &QPushButton::clicked, this, [i, this, chatButton]() {
              qDebug() << "Выбран чат:" << i;
              // Здесь логика запроса 
       });

       scrollLayout->addWidget(chatButton);
       }

         // Добавляем растягивающийся элемент в конец, чтобы кнопки не растягивались
  scrollLayout->addStretch();

  ui->messButtonPush->setText("Отправить 📨");

  //////////////////////////////////////////////////////////////
  /// Тестовые сообщения
  /////////////////////////////////////////////////////////////

       // Настройка правой панели с сообщениями
       QWidget *scrollContentMessages = ui->scrollAreaWidgetContents;
       QVBoxLayout *scrollLayoutMessages = new QVBoxLayout(scrollContentMessages);
       scrollLayoutMessages->setAlignment(Qt::AlignTop);
       scrollLayoutMessages->setSpacing(5); // Расстояние между сообщениями
       scrollLayoutMessages->setContentsMargins(8, 8, 8, 8); // Отступы от краев
       scrollContentMessages->setLayout(scrollLayoutMessages);

              // Добавляем 50 тестовых сообщений
       for (int i = 1; i <= 50; ++i) {
              bool isMyMessage = (i % 2 == 0);
              
              // вместо messageWidget->setProperty("class", isMyMessage ? "message-widget-my" : "message-widget-other");
              QWidget *messageWidget = new QWidget(scrollContentMessages);
              QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);
              messageLayout->setContentsMargins(0, 0, 0, 0);

              // Контент сообщения — именно его будем стилизовать как "bubble"
              QWidget *contentWidget = new QWidget(messageWidget);
              contentWidget->setObjectName(isMyMessage ? "message-bubble-my" : "message-bubble-other");

              // ВАЖНО: разрешаем рисовать фон у этого виджета
              contentWidget->setAttribute(Qt::WA_StyledBackground, true); // <-- эта строка решает проблему с фоном
              // альтернативно: contentWidget->setAutoFillBackground(true);
              // contentWidget->setStyleSheet("background: red;"); // для теста — должен стать красным
              QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
              contentLayout->setContentsMargins(12, 8, 12, 8); // padding внутри bubble
              contentLayout->setSpacing(4);

              // Текст сообщения
              QLabel *messageText = new QLabel(contentWidget);
              messageText->setObjectName(isMyMessage ? "message-text-my" : "message-text");
              messageText->setWordWrap(true);
              messageText->setText(QString("Тестовое сообщение %1\nЭто длинный текст ...").arg(i));
              messageText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
              messageText->setAlignment(Qt::AlignLeft | Qt::AlignTop);

              // Время
              QLabel *timeLabel = new QLabel(contentWidget);
              timeLabel->setObjectName(isMyMessage ? "message-time-my" : "message-time");
              timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
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
}

MainWindow::~MainWindow () { delete ui; }


MainWindow *MainWindow::createClient()
{
  StartScreen s;                  //   СОЗДАЕТСЯ StartScreen
  auto result = s.exec();            //   ПОКАЗЫВАЕТСЯ StartScreen (модально)
  if(result == QDialog::Rejected)
  {
    return nullptr;
  }
  // Дальше создается MainWindow после успешного входа (запуск уже в Main)
  auto w = new MainWindow();
  w->setAttribute(Qt::WA_DeleteOnClose); //Удалит если закроем!!
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
}

