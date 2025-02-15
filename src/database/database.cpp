#include "database/database.h"
#include <iostream>

database_t::database_t() {
    if (sqlite3_open("passwords.db", &m_db) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(m_db) << std::endl;
    }
}

database_t::~database_t() {
    sqlite3_close(m_db);
}

/**
 * @brief Создает таблицу, если ее нет.
 */
void database_t::init_database_() {
    const char* sql = "CREATE TABLE IF NOT EXISTS passwords ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "title TEXT NOT NULL, "
                      "url TEXT NOT NULL, "
                      "username TEXT NOT NULL, "
                      "password BLOB NOT NULL, "
                      "notes TEXT NOT NULL);";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

/**
 * @brief Добавляет запись в базу данных.
 */
bool database_t::add_entry_(const std::string& title, const std::string& url,
                            const std::string& username, const std::string& password,
                            const std::string& notes, const std::string& masterPassword) {
    std::vector<unsigned char> key = m_encryption.derive_key_(masterPassword);
    std::vector<unsigned char> encryptedPassword = m_encryption.encrypt_aes_(password, key);

    const char* sql = "INSERT INTO passwords (title, url, username, password, notes) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing insert statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4, encryptedPassword.data(), encryptedPassword.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, notes.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

/**
 * @brief Поиск записей по запросу.
 */
std::vector<password_entry_t> database_t::search_entries_(const std::string& query, const std::string& masterPassword) {
    std::vector<password_entry_t> results;
    const char* sql = "SELECT id, title, url, username, password, notes FROM passwords "
                      "WHERE title LIKE ? OR url LIKE ? OR username LIKE ? OR notes LIKE ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing search statement: " << sqlite3_errmsg(m_db) << std::endl;
        return results;
    }

    std::string likeQuery = "%" + query + "%";
    for (int i = 1; i <= 4; i++) {
        sqlite3_bind_text(stmt, i, likeQuery.c_str(), -1, SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        password_entry_t entry;
        entry.m_id = sqlite3_column_int(stmt, 0);
        entry.m_title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.m_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.m_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const unsigned char* encryptedPassword = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 4));
        int passwordSize = sqlite3_column_bytes(stmt, 4);
        entry.m_encryptedPassword.assign(encryptedPassword, encryptedPassword + passwordSize);

        entry.m_notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        results.push_back(entry);
    }

    sqlite3_finalize(stmt);
    return results;
}

/**
 * @brief Удаляет запись по ID.
 */
bool database_t::delete_entry_(int id) {
    const char* sql = "DELETE FROM passwords WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing delete statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

