# Рефакторинг проектов [Chat](https://github.com/DmitryVic/Chat-make-build), [Chat](https://github.com/DmitryVic/Chat-HW-Project), внедрение БД MySQL

**Студент** - Зайкин Дмитрий
**Группа** - CPLUS-68


---

# Описание проекта
Проект чата разделен на серверную и клиентскую части. Проект реализован для ОС Linux (для кроссплатформенности клиенской части требуется откорректировать класс Network). Взаимодействие программ осуществляется по сети (TCP): настройка IP-адреса указана ниже по тексту. Для хранения информации серверная часть приложения использует базу данных MySQL.

---
Кодировка проекта: UTF-8

❗ Перед использованием клиентской части приложения установить в терминале кодировку UTF-8 (перед подключением из Windows к Linux по SSH выполнить команду: `chcp 65001`).

# Структурная схема приложения
![Структурная схема приложения](CHAT.jpg)

# Предварительная настройка, сборка и компиляция проекта

## Установка и настройка БД MySQL на серверной части (ОС: Linux)

### Выполнить установку БД MySQL на сервере:
```bash
sudo apt-get update
sudo apt-get install mysql-server
```
### Настройка БД:
Открыть MySQL в терминале:
```bash
sudo mysql
```

Добавление БД:
```sql
CREATE DATABASE IF NOT EXISTS chat
CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;
```

Cоздать **пользователя БД** для входа `ALL PRIVILEGES`:
```sql
CREATE USER 'chatchat_user'@'localhost' IDENTIFIED BY '12345678';
GRANT ALL PRIVILEGES ON *.* TO 'chat_user'@'localhost';
FLUSH PRIVILEGES;

CREATE USER 'chat_user'@'127.0.0.1' IDENTIFIED BY '12345678';
GRANT ALL PRIVILEGES ON *.* TO 'chat_user'@'127.0.0.1';
FLUSH PRIVILEGES;
```

Выход:
```sql
\q
```

Перезапуск БД:
```bash
sudo systemctl restart mysql
```
Выполнить проверку доступа к MySQL локально на сервере:
```bash
mysql -u chat_user -p -h 127.0.0.1
```

### ❗ **Данные созданного пользователя и название БД должны совпадать со значениями полей класса** `DataBaseMySQL` в файле `BD_MySQL.h`:
- `SQL_USER` - пользователь;
- `SQL_PASS` - пароль;
- `SQL_BD` - база данных.

## Установка дополнительных библиотек

### Выполнить установку библиотеки JSON `json.hpp`
Для ОС: Linux Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install nlohmann-json3-dev
```

❗**Важно**: унификация версий JSON для сетевого взаимодействия:

Клиентская и серверная части взаимодействуют по сети (TCP) и используют JSON для сериализации данных, на всех компьютерах должна использоваться одинаковая минимальная версия библиотеки `nlohmann/json`.

В `CMakeLists.txt` заменить номер версии на минимальную:

Как проверить версию на Linux:
```bash
dpkg -s nlohmann-json3-dev | grep Version
Version: 3.7.3-1
```
В `CMakeLists.txt` указать эту версию:
```bash
find_package(nlohmann_json 3.7.3 REQUIRED)
```

### Выполнить установку библиотеки `mysql.h`
Для ОС: Linux Ubuntu/Debian:
```bash
sudo apt update
sudo apt install libmysqlclient-dev
sudo apt-get install pkg-config
```
Предполагаемые пути установки файлов:
- Заголовочные файлы: `/usr/include/mysql/` (`mysql.h`)
- Библиотека: `libmysqlclient.so` `/usr/lib/x86_64-linux-gnu/`

Проверка наличия файлов (`mysql.h`, `libmysqlclient.so`):
```bash
ls /usr/include/mysql/ | grep "mysql.h"
ls /usr/lib/x86_64-linux-gnu | grep "libmysqlclient.so"
```
## Настройка IP адресса

Взаимодействие программ осуществляется по сети (TCP). IP адресс сервера задается в обьекте `network` класса `NetworkClient` файл `main.cpp` клиенской части (`/client/src/main.cpp`).

# Сборка и компиляция проекта CMake

## Сборка Linux (сервер + клиент)
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Сборка только серверной части:
```bash
mkdir build
cd build
make chat_server -j$(nproc)
```

## Сборка только серверной части:
```bash
cd build
make chat_client -j$(nproc)
```

## Очистка:
```bash
make clean
```

## Запуск:
```bash
# Сервер
./build/server/chat_server

# Клиент
./build/client/chat_client
```