#include "interface/tui.h"
#include "database/database.h"

#include <iostream>
#include <limits>
#include <iomanip>   // для setw и т.д.
#include <sstream>

/**
 * @brief Заглушка для копирования пароля в буфер обмена.
 */
static void copy_password_to_clipboard(const std::string& password) {
    // TODO: Реализовать платформенно-зависимую логику
    std::cout << "(Simulation) Copy password to clipboard: " << password << "\n";
}

/**
 * @brief Печатает «шапку» для табличного вывода записей.
 */
static void print_table_header() {
    std::cout << "\n=== Results ===\n";
    std::cout << std::left
              << std::setw(4)  << "#"       
              << std::setw(6)  << "ID"
              << std::setw(15) << "Title"
              << std::setw(20) << "URL"
              << std::setw(15) << "Username"
              << "Notes"
              << "\n";
    // Линия-разделитель (количество символов можно подстроить):
    std::cout << std::string(70, '-') << "\n";
}

/**
 * @brief Печатает строку «таблицы» для одной записи.
 * @param index Порядковый номер (1-based).
 * @param entry Структура записи (без расшифрованного пароля).
 */
static void print_table_row(int index, const password_entry_t& entry) {
    std::cout << std::left
              << std::setw(4)  << (std::to_string(index) + ")")
              << std::setw(6)  << entry.m_id
              << std::setw(15) << entry.m_title
              << std::setw(20) << entry.m_url
              << std::setw(15) << entry.m_username
              << entry.m_notes
              << "\n";
}

/**
 * @brief Печатает единственную запись в табличном стиле (шапка + одна строка).
 * @param entry запись, которую нужно вывести
 * @param index порядковый номер (часто можно передать 1)
 */
static void print_single_entry_as_table(const password_entry_t& entry, int index = 1) {
    print_table_header();
    print_table_row(index, entry);
    std::cout << std::endl; // пустая строка в конце
}

/**
 * @brief Меню для выбранной записи (операции над одной записью).
 * @param db Ссылка на объект базы данных
 * @param masterPassword Мастер-пароль
 * @param entryId ID записи, которую хотим просмотреть/редактировать
 */
static void handle_entry_menu(database_t& db,
                              const std::string& masterPassword,
                              int entryId) 
{
    // Сразу получаем запись из базы:
    password_entry_t entry = db.get_entry_by_id_(entryId);
    if (entry.m_id == 0) {
        std::cout << "Error: entry not found (ID=" << entryId << ").\n";
        return;
    }

    while (true) {
        std::cout << "\n=== Entry Menu ===\n";
        // Выводим запись «в таблицу» (одна строка)
        print_single_entry_as_table(entry, 1);

        std::cout << "Actions:\n"
                  << "1) Delete\n"
                  << "2) Edit\n"
                  << "3) Show Decrypted Password\n"
                  << "4) Copy Password\n"
                  << "5) Back\n"
                  << "Choose: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            // Удаление записи
            if (db.delete_entry_(entryId)) {
                std::cout << "Entry deleted successfully.\n";
            } else {
                std::cout << "Failed to delete entry.\n";
            }
            break; // выходим в предыдущее меню

        } else if (choice == 2) {
            // Редактирование
            std::string newTitle, newUrl, newUsername, newPass, newNotes;
            std::cout << "Enter new title (leave empty to keep old): ";
            std::getline(std::cin, newTitle);
            std::cout << "Enter new URL (leave empty to keep old): ";
            std::getline(std::cin, newUrl);
            std::cout << "Enter new username (leave empty to keep old): ";
            std::getline(std::cin, newUsername);
            std::cout << "Enter new password (leave empty to keep old): ";
            std::getline(std::cin, newPass);
            std::cout << "Enter new notes (leave empty to keep old): ";
            std::getline(std::cin, newNotes);

            bool ok = db.update_entry_(entryId, newTitle, newUrl,
                                       newUsername, newPass, newNotes,
                                       masterPassword);
            if (ok) {
                std::cout << "Entry updated.\n";
                // Заново получаем запись, чтобы отобразить актуальные данные
                entry = db.get_entry_by_id_(entryId);
            } else {
                std::cout << "Failed to update entry.\n";
            }

        } else if (choice == 3) {
            // Показать расшифрованный пароль (с ожиданием 'q')
            std::string decrypted = db.get_decrypted_password_(entryId, masterPassword);
            if (!decrypted.empty()) {
                std::cout << "Decrypted password: " << decrypted << "\n";
                std::cout << "Press 'q' to go back: ";
                while (true) {
                    std::string input;
                    std::cin >> input;
                    if (input == "q" || input == "Q") {
                        break;
                    }
                    std::cout << "Press 'q' to go back: ";
                }
            } else {
                std::cout << "Could not decrypt or entry not found.\n";
            }

        } else if (choice == 4) {
            // Копирование пароля в «буфер обмена» (пока заглушка)
            std::string decrypted = db.get_decrypted_password_(entryId, masterPassword);
            if (!decrypted.empty()) {
                copy_password_to_clipboard(decrypted);
            } else {
                std::cout << "Could not decrypt or entry not found.\n";
            }

        } else if (choice == 5) {
            // Возврат в предыдущее меню
            break;

        } else {
            std::cout << "Invalid choice!\n";
        }
    }
}

/**
 * @brief Функция для добавления новой записи (через ввод с консоли).
 */
static void handle_add_entry(database_t& db, const std::string& masterPassword) {
    // На случай, если в буфере остался лишний ввод
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string title, url, username, password, notes;
    std::cout << "Title: ";
    std::getline(std::cin, title);
    std::cout << "URL: ";
    std::getline(std::cin, url);
    std::cout << "Username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);
    std::cout << "Notes: ";
    std::getline(std::cin, notes);

    bool added = db.add_entry_(title, url, username, password, notes, masterPassword);
    if (added) {
        std::cout << "Entry added successfully.\n";
    } else {
        std::cout << "Failed to add entry.\n";
    }
}

/**
 * @brief Универсальная функция для вывода списка записей в виде таблицы
 *        и выбора одной записи по вводу (1-based index или 'q').
 * @param results Список записей
 * @return ID выбранной записи или 0, если пользователь ввёл 'q' или некорректный индекс
 */
static int pick_entry_from_list(const std::vector<password_entry_t>& results) {
    // Печатаем «табличный» заголовок:
    print_table_header();
    // Печатаем каждую запись:
    for (size_t i = 0; i < results.size(); ++i) {
        print_table_row((int)(i + 1), results[i]);
    }

    std::cout << "\nSelect an entry (1-" << results.size()
              << ") or 'q' to go back: ";
    std::string input;
    std::cin >> input;
    if (input == "q" || input == "Q") {
        return 0; // сигнал о возврате назад
    }

    int idx;
    try {
        idx = std::stoi(input);
    } catch (...) {
        std::cout << "Invalid input!\n";
        return 0;
    }
    idx -= 1;
    if (idx < 0 || idx >= (int)results.size()) {
        std::cout << "Invalid choice!\n";
        return 0;
    }

    // Возвращаем ID выбранной записи:
    return results[idx].m_id; 
}

/**
 * @brief Обработчик пункта "Search Entry" главного меню.
 */
static void handle_search(database_t& db, const std::string& masterPassword) {
    // Очистим буфер
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter search query: ";
    std::string query;
    std::getline(std::cin, query);

    // Запрашиваем все подходящие записи
    auto results = db.search_entries_(query, masterPassword);
    if (results.empty()) {
        std::cout << "No entries found.\n";
        return;
    }

    // Даём пользователю выбрать
    int entryId = pick_entry_from_list(results);
    if (entryId != 0) {
        // Если выбрана конкретная запись - открываем меню записи
        handle_entry_menu(db, masterPassword, entryId);
    }
}

/**
 * @brief Показывает все записи из базы (аналогично "search" с пустым запросом).
 */
static void handle_view_all(database_t& db, const std::string& masterPassword) {
    auto results = db.search_entries_("", masterPassword);
    if (results.empty()) {
        std::cout << "Database is empty.\n";
        return;
    }

    int entryId = pick_entry_from_list(results);
    if (entryId != 0) {
        handle_entry_menu(db, masterPassword, entryId);
    }
}

/**
 * @brief Основное меню TUI.
 */
void start_tui(database_t& db, const std::string& masterPassword) {
    while (true) {
        std::cout << "\n=== Main Menu ===\n"
                  << "1) Add Entry\n"
                  << "2) Search Entry\n"
                  << "3) View All Entries\n"
                  << "4) Exit\n"
                  << "Choose: ";

        int choice;
        std::cin >> choice;
        if (!std::cin.good()) {
            // Некорректный ввод (строка или символ), очищаем буфер
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
        case 1:
            handle_add_entry(db, masterPassword);
            break;
        case 2:
            handle_search(db, masterPassword);
            break;
        case 3:
            handle_view_all(db, masterPassword);
            break;
        case 4:
            std::cout << "Exiting...\n";
            return;
        default:
            std::cout << "Invalid choice!\n";
        }
    }
}

