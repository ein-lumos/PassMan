cmake_minimum_required(VERSION 3.14)
project(PassMan)

set(CMAKE_CXX_STANDARD 17)

# Подключаем OpenSSL и SQLite
find_package(OpenSSL REQUIRED)

# Указываем пути к исходникам и заголовочным файлам
include_directories(include config)

# Компиляция библиотеки шифрования
add_library(encryption STATIC
    src/encryption/encryption.cpp
)

target_link_libraries(encryption OpenSSL::Crypto)

# Компиляция библиотеки базы данных
add_library(database STATIC
    src/database/database.cpp
)

target_link_libraries(database encryption sqlite3)

# Компиляция теста для шифрования
add_executable(test_encryption tests/test_encryption.cpp)
target_link_libraries(test_encryption encryption OpenSSL::Crypto)

# Компиляция теста для базы данных
add_executable(test_database tests/test_database.cpp)
target_link_libraries(test_database database encryption sqlite3)

