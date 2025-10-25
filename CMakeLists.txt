cmake_minimum_required(VERSION 3.10)
project(Chat)



# СПринудительно установить стандарт c++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Настройка локали WIN32 для MINGW
if(WIN32)
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
elseif(UNIX)
    add_definitions(-DSET_GLOBAL_LOCALE_LINUX)
endif()

# Подключаем подпроекты
add_subdirectory(common)
add_subdirectory(server)
# add_subdirectory(client)

# Поиск библиотеки json
find_package(nlohmann_json 3.7.3 REQUIRED)