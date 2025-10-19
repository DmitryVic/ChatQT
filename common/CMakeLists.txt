

# Создаем статическую библиотеку
add_library(common STATIC
    src/Message.cpp
    src/User.cpp
)

# Публичные заголовки - видны ВСЕМ, кто использует библиотеку
target_include_directories(common PUBLIC 
    include   # Путь к папке с публичными заголовками
)
