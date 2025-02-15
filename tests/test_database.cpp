#include "database/database.h"
#include <iostream>

int main() {
    database_t db;
    db.init_database_();

    std::string masterPassword;
    std::cout << "Enter master password: ";
    std::getline(std::cin, masterPassword);

    while (true) {
        std::cout << "\n1) Add Entry"
                  << "\n2) Search"
                  << "\n3) Delete Entry"
                  << "\n4) Show Decrypted Password by ID"
                  << "\n5) Exit"
                  << "\nChoose: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // очистка буфера ввода

        if (choice == 1) {
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

        } else if (choice == 2) {
            std::string query;
            std::cout << "Search query: ";
            std::getline(std::cin, query);

            auto results = db.search_entries_(query, masterPassword);
            std::cout << "=== Search results ===\n";
            for (auto& entry : results) {
                // Можно выводить и другие поля
                std::cout << "ID: " << entry.m_id << ", Title: " << entry.m_title
                          << ", Username: " << entry.m_username << "\n";
            }

        } else if (choice == 3) {
            int id;
            std::cout << "Enter ID to delete: ";
            std::cin >> id;
            std::cin.ignore();

            if (db.delete_entry_(id)) {
                std::cout << "Entry deleted successfully.\n";
            } else {
                std::cout << "Failed to delete entry.\n";
            }

        } else if (choice == 4) {
            int id;
            std::cout << "Enter ID to show decrypted password: ";
            std::cin >> id;
            std::cin.ignore();

            std::string decrypted = db.get_decrypted_password_(id, masterPassword);
            if (!decrypted.empty()) {
                std::cout << "Decrypted password: " << decrypted << std::endl;
            } else {
                std::cout << "No password found for ID " << id << " or could not decrypt.\n";
            }

        } else if (choice == 5) {
            std::cout << "Exiting...\n";
            break;

        } else {
            std::cout << "Invalid option!\n";
        }
    }

    return 0;
}

