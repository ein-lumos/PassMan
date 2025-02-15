cmake_minimum_required(VERSION 3.14)
project(PassMan)

set(CMAKE_CXX_STANDARD 17)

# Ищем OpenSSL (Crypto)
find_package(OpenSSL REQUIRED)

# Если нужно искать sqlite3 через find_package:
# find_package(SQLite3 REQUIRED)
# include_directories(${SQLite3_INCLUDE_DIRS})

# Подключаем заголовочные файлы (include и config)
include_directories(include config)

# Библиотека шифрования
add_library(encryption STATIC
    src/encryption/encryption.cpp
)
target_link_libraries(encryption OpenSSL::Crypto)

# Библиотека базы данных
add_library(database STATIC
    src/database/database.cpp
)
target_link_libraries(database encryption sqlite3)

# Исполняемый файл для TUI-приложения
add_executable(passman
    src/main.cpp
    src/interface/tui.cpp
)
target_link_libraries(passman
    database         # Наша библиотека работы с БД
    encryption       # Библиотека шифрования
    sqlite3          # Системная библиотека SQLite3
    OpenSSL::Crypto  # OpenSSL
)

# Тест для шифрования
add_executable(test_encryption
    tests/test_encryption.cpp
)
target_link_libraries(test_encryption
    encryption
    OpenSSL::Crypto
)

# Тест для базы данных
add_executable(test_database
    tests/test_database.cpp
)
target_link_libraries(test_database
    database
    encryption
    sqlite3
)

