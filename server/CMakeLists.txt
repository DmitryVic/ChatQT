# компиляция только для Linux
if(UNIX)
    # Явное указание путей для MySQL
    set(MYSQL_INCLUDE_DIR "/usr/include/mysql")
    set(MYSQL_LIBRARY "/usr/lib/x86_64-linux-gnu/libmysqlclient.so")

    # Проверка существования файлов
    if(NOT EXISTS ${MYSQL_INCLUDE_DIR}/mysql.h)
        message(FATAL_ERROR "MySQL headers not found in ${MYSQL_INCLUDE_DIR}")
    endif()

    if(NOT EXISTS ${MYSQL_LIBRARY})
        message(FATAL_ERROR "MySQL client library not found at ${MYSQL_LIBRARY}")
    endif()

    # СПринудительно установить стандарт c++ 17
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(Threads REQUIRED)

    add_executable(chat_server
        src/dataUserOnline.cpp    
        src/main.cpp
        src/Core.cpp
        src/DB_MySQL.cpp
        src/NetworkServer.cpp
        src/MessageHandler.cpp
        src/Logger.cpp
    )

    target_compile_definitions(chat_server PRIVATE SET_GLOBAL_LOCALE_LINUX)
    
    # линковка с общей библиотекой
    #target_link_libraries(chat_server PRIVATE common ${MYSQL_LIBRARY})
    target_link_libraries(chat_server PRIVATE common ${MYSQL_LIBRARY} Threads::Threads)

    #Добавляем пути к  заголовкам сервера
    target_include_directories(chat_server PRIVATE 
        include
        ${MYSQL_INCLUDE_DIR}
    )

else()
    message(WARNING "Компиляция без серверной части, так как она только для Linux")
endif()