#ifndef TUI_H
#define TUI_H

#include <string>

// Вперёд объявляем класс database_t (чтобы не включать весь database.h)
class database_t;

/**
 * @brief Запускает TUI с основным меню.
 * @param db Ссылка на объект базы данных
 * @param masterPassword Мастер-пароль для операций шифрования/дешифрования
 */
void start_tui(database_t& db, const std::string& masterPassword);

#endif // TUI_H

