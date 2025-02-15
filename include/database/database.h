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
    std::vector<unsigned char> m_encryptedPassword; // зашифрованный пароль
    std::string m_notes;
};

class database_t {
private:
    sqlite3* m_db;
    encryption_t m_encryption;

public:
    database_t();
    ~database_t();

    void init_database_();

    bool add_entry_(const std::string& title,
                    const std::string& url,
                    const std::string& username,
                    const std::string& password,
                    const std::string& notes,
                    const std::string& masterPassword);

    std::vector<password_entry_t> search_entries_(const std::string& query,
                                                  const std::string& masterPassword);

    bool delete_entry_(int id);

    /**
     * @brief Возвращает расшифрованный пароль для записи с указанным ID.
     */
    std::string get_decrypted_password_(int id, const std::string& masterPassword);

    /**
     * @brief Обновляет запись (title, url, username, password, notes)
     *        Если поле пустое, сохраняется старое значение.
     */
    bool update_entry_(int id,
                       const std::string& newTitle,
                       const std::string& newUrl,
                       const std::string& newUsername,
                       const std::string& newPassword,
                       const std::string& newNotes,
                       const std::string& masterPassword);

    /**
     * @brief Получает одну запись по ID (ID уникален).
     * @return Найденная запись или запись с m_id = 0, если нет в БД.
     */
    password_entry_t get_entry_by_id_(int id);
};

#endif // DATABASE_H

