#ifndef DATABASE_H
#define DATABASE_H

#include "encryption/encryption.h"
#include <vector>
#include <string>
#include <sqlite3.h>

/**
 * @brief Структура записи в базе данных.
 */
struct password_entry_t {
    int m_id;
    std::string m_title;
    std::string m_url;
    std::string m_username;
    std::vector<unsigned char> m_encryptedPassword;
    std::string m_notes;
};

/**
 * @brief Класс для работы с базой данных.
 */
class database_t {
private:
    sqlite3* m_db;
    encryption_t m_encryption;

public:
    database_t();
    ~database_t();

    void init_database_();
    bool add_entry_(const std::string& title, const std::string& url,
                    const std::string& username, const std::string& password,
                    const std::string& notes, const std::string& masterPassword);
    std::vector<password_entry_t> search_entries_(const std::string& query, const std::string& masterPassword);
    bool delete_entry_(int id);
};

#endif // DATABASE_H

