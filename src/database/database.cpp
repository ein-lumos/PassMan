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

void database_t::init_database_() {
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS passwords ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "url TEXT NOT NULL, "
        "username TEXT NOT NULL, "
        "password BLOB NOT NULL, "
        "notes TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool database_t::add_entry_(
    const std::string& title,
    const std::string& url,
    const std::string& username,
    const std::string& password,
    const std::string& notes,
    const std::string& masterPassword
) {
    // Генерируем ключ и шифруем пароль
    std::vector<unsigned char> key = m_encryption.derive_key_(masterPassword);
    std::vector<unsigned char> encryptedPassword = m_encryption.encrypt_aes_(password, key);

    const char* sql = "INSERT INTO passwords (title, url, username, password, notes) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing insert statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4, encryptedPassword.data(), (int)encryptedPassword.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, notes.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<password_entry_t> database_t::search_entries_(
    const std::string& query,
    const std::string& /* masterPassword */
) {
    std::vector<password_entry_t> results;
    const char* sql = 
        "SELECT id, title, url, username, password, notes FROM passwords "
        "WHERE title LIKE ? OR url LIKE ? OR username LIKE ? OR notes LIKE ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing search statement: " << sqlite3_errmsg(m_db) << std::endl;
        return results;
    }

    std::string likeQuery = "%" + query + "%";
    for (int i = 1; i <= 4; ++i) {
        sqlite3_bind_text(stmt, i, likeQuery.c_str(), -1, SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        password_entry_t entry;
        entry.m_id = sqlite3_column_int(stmt, 0);
        entry.m_title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.m_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.m_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const unsigned char* encryptedData = 
            reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 4));
        int dataSize = sqlite3_column_bytes(stmt, 4);
        entry.m_encryptedPassword.assign(encryptedData, encryptedData + dataSize);

        entry.m_notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        results.push_back(entry);
    }

    sqlite3_finalize(stmt);
    return results;
}

bool database_t::delete_entry_(int id) {
    const char* sql = "DELETE FROM passwords WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing delete statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::string database_t::get_decrypted_password_(int id, const std::string& masterPassword) {
    std::string decrypted;
    const char* sql = "SELECT password FROM passwords WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing get password statement: " << sqlite3_errmsg(m_db) << std::endl;
        return "";
    }

    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* data = 
            reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 0));
        int size = sqlite3_column_bytes(stmt, 0);

        std::vector<unsigned char> encryptedData(data, data + size);
        std::vector<unsigned char> key = m_encryption.derive_key_(masterPassword);

        decrypted = m_encryption.decrypt_aes_(encryptedData, key);
    }

    sqlite3_finalize(stmt);
    return decrypted;
}

bool database_t::update_entry_(
    int id,
    const std::string& newTitle,
    const std::string& newUrl,
    const std::string& newUsername,
    const std::string& newPassword,
    const std::string& newNotes,
    const std::string& masterPassword
) {
    // 1) Сначала прочитаем текущие данные
    const char* selectSql = "SELECT title, url, username, password, notes FROM passwords WHERE id = ?;";
    sqlite3_stmt* selectStmt = nullptr;

    if (sqlite3_prepare_v2(m_db, selectSql, -1, &selectStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing select statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    sqlite3_bind_int(selectStmt, 1, id);

    std::string oldTitle, oldUrl, oldUsername, oldNotes;
    std::vector<unsigned char> oldEncryptedPass;

    if (sqlite3_step(selectStmt) == SQLITE_ROW) {
        oldTitle = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 0));
        oldUrl = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 1));
        oldUsername = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 2));

        const unsigned char* data = 
            reinterpret_cast<const unsigned char*>(sqlite3_column_blob(selectStmt, 3));
        int size = sqlite3_column_bytes(selectStmt, 3);
        oldEncryptedPass.assign(data, data + size);

        oldNotes = reinterpret_cast<const char*>(sqlite3_column_text(selectStmt, 4));
    } else {
        // Записи с таким ID нет
        sqlite3_finalize(selectStmt);
        return false;
    }

    sqlite3_finalize(selectStmt);

    // 2) Если новое поле пустое, используем старое
    std::string finalTitle = newTitle.empty() ? oldTitle : newTitle;
    std::string finalUrl = newUrl.empty() ? oldUrl : newUrl;
    std::string finalUsername = newUsername.empty() ? oldUsername : newUsername;
    std::string finalNotes = newNotes.empty() ? oldNotes : newNotes;

    // 3) Пароль - если пустой, не меняем
    std::vector<unsigned char> finalEncryptedPass;
    if (newPassword.empty()) {
        // Оставляем зашифрованный
        finalEncryptedPass = oldEncryptedPass;
    } else {
        // Перешифровываем
        std::vector<unsigned char> key = m_encryption.derive_key_(masterPassword);
        finalEncryptedPass = m_encryption.encrypt_aes_(newPassword, key);
    }

    // 4) Выполним UPDATE
    const char* updateSql = 
        "UPDATE passwords SET title = ?, url = ?, username = ?, password = ?, notes = ? WHERE id = ?;";
    sqlite3_stmt* updateStmt = nullptr;
    if (sqlite3_prepare_v2(m_db, updateSql, -1, &updateStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing update statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_text(updateStmt, 1, finalTitle.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(updateStmt, 2, finalUrl.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(updateStmt, 3, finalUsername.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(updateStmt, 4, finalEncryptedPass.data(), (int)finalEncryptedPass.size(), SQLITE_STATIC);
    sqlite3_bind_text(updateStmt, 5, finalNotes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(updateStmt, 6, id);

    bool success = (sqlite3_step(updateStmt) == SQLITE_DONE);
    sqlite3_finalize(updateStmt);
    return success;
}

password_entry_t database_t::get_entry_by_id_(int id) {
    password_entry_t entry{};
    entry.m_id = 0; // Укажем 0, пока не найдём

    const char* sql = "SELECT id, title, url, username, password, notes "
                      "FROM passwords WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing get_entry_by_id statement: "
                  << sqlite3_errmsg(m_db) << std::endl;
        return entry;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entry.m_id = sqlite3_column_int(stmt, 0);
        entry.m_title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.m_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.m_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const unsigned char* data =
            reinterpret_cast<const unsigned char*>(sqlite3_column_blob(stmt, 4));
        int size = sqlite3_column_bytes(stmt, 4);
        entry.m_encryptedPassword.assign(data, data + size);

        entry.m_notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    return entry;
}

