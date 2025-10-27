cmake_minimum_required(VERSION 3.16)

project(Admin_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#опция CMAKE_INCLUDE_CURRENT_DIR обязательно должна быть включена,
#если вы не меняли пути генерации файлов для Qt.
#при использовании .ui файлов, Qt на их основе генерирует промежуточные
#файлы исходников, и будет складывать их в директории сборки.
#Чтобы среда их видела, необходимо добавить в директории для включения
#(include, директории, хранящие подключаемые исходники), директорию сборки.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

#здесь мы создаем переменную CHAT_LIBRARIES, в которой будем хранить
#все подключаемые к проекту библиотеки, в т.ч. и библиотеки Qt
set(CHAT_LIBRARIES)

#здесь мы создаем и заполняем список подключаемых модулей Qt
#если нужно добавить модули, добавьте их в этот список по образцу:
#set(Qt5Modules [модуль 1] [модуль 2] ... [модуль n])
set(Qt5Modules Core Widgets)

#здесь мы формируем из списка модулей список библиотек,
#которые требуется подключить, и складываем их в CHAT_LIBRARIES
foreach(QtModule IN LISTS Qt5Modules)
  list(APPEND CHAT_LIBRARIES Qt5::${QtModule})
endforeach(QtModule)

#здесь проверяется наличие всех необходимых модулей и компонентов Qt
find_package(Qt5 COMPONENTS ${Qt5Modules} REQUIRED LinguistTools)

# СЕТЕВАЯ ЧАСТЬ ИЗ КОНСОЛЬНОЙ ВЕРСИИ (ОСНОВА)
if(WIN32)
    set(NETWORK_CLIENT_SRC NetworkClientWin.cpp)
else()
    set(NETWORK_CLIENT_SRC NetworkClientLinux.cpp)
endif()

# Исходники из консольной версии (в корне)
set(CONSOLE_SRC_FILES

    MessageHandler.cpp
    ${NETWORK_CLIENT_SRC}
    UserStatus.cpp
    Logger.cpp
)

# Qt UI исходники (лежат в ui папке)
file(GLOB_RECURSE UI_SRC_FILES ui/*.cpp)
file(GLOB_RECURSE UI_HEADER_FILES ui/*.h)
file(GLOB_RECURSE UI_FILES ui/*.ui)

# Ресурсы и переводы (если есть)
file(GLOB_RECURSE RC_FILES resources/*.qrc)

#здесь мы включаем обязательные для нас опции
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#вызываем обработку .ui и .qrc файлов. Здесь же происходит генерация
#промежуточных исходников.
qt5_wrap_ui(QUI ${UI_FILES})
qt5_add_resources(QRC ${RC_FILES})

#здесь происходит создание файлов переводов и установка зависимостей,
#чтобы эти файлы генерировались во время билда проекта
set(TS_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/admin_translations/Admin_client_ru_RU.ts)
file(GLOB_RECURSE TS_SOURCES "*.cpp" "*.h" "*.ui")
add_custom_target(admin_translations)
#по сути, при билде для каждого .ts файла вызывается
#утилита lupdate.exe
foreach(_ts_file ${TS_FILES})
  get_filename_component(_ts_name ${_ts_file} NAME_WE)
  add_custom_command(
    OUTPUT "${_ts_file}"
    DEPENDS ${TS_SOURCES}
#вот тут как раз описывается путь к вызываемой утилите lupdate.exe
#и ее входные аргументы:
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive
    ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
    )
  add_custom_target(translate_${_ts_name} DEPENDS ${_ts_file})
  add_dependencies(admin_translations translate_${_ts_name})
endforeach()

#подключаем файлы трансляции(переводов)
qt5_add_translation(QM_FILES ${TS_FILES})

# СОЗДАЕМ ИСПОЛНЯЕМЫЙ ФАЙЛ (ОСНОВА ИЗ КОНСОЛЬНОЙ ВЕРСИИ + Qt UI)
add_executable(Admin_client
    # Главный файл (должен быть в корне)
    main.cpp
    
    # Консольные исходники (все файлы из корня)
    ${CONSOLE_SRC_FILES}
    
    # Qt UI файлы
    ${UI_SRC_FILES}
    ${UI_HEADER_FILES}
    ${UI_FILES}
    ${RC_FILES}
    
    # Переводы
    ${QM_FILES}
)

# Отключаем консоль для всех платформ через свойства Qt
set_target_properties(Admin_client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# СЕТЕВЫЕ НАСТРОЙКИ ИЗ КОНСОЛЬНОЙ ВЕРСИИ
# Для Windows-клиента
if(WIN32)
    target_link_libraries(Admin_client PRIVATE ws2_32)
endif()

# Для Linux-клиента
if(UNIX AND NOT APPLE)
    target_compile_definitions(Admin_client PRIVATE SET_GLOBAL_LOCALE_LINUX)
endif()

# линкует клиентский исполняемый файл с общей библиотекой
target_link_libraries(Admin_client PRIVATE common)
# общая библиотека нужна только для сборки PRIVATE

# ПОДКЛЮЧАЕМ QT БИБЛИОТЕКИ
target_link_libraries(Admin_client
  PUBLIC
  ${CHAT_LIBRARIES}
  )


# ДИРЕКТОРИИ ДЛЯ INCLUDE
# Добавляем директорию с Qt исходниками в include path, чтобы
# сгенерированные ui_*.h могли найти заголовки типа "loginform.h".
target_include_directories(Admin_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}  # Добавляем корень для заголовков в корне
)