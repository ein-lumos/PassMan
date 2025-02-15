#include "database/database.h"
#include <iostream>

int main() {
    database_t db;
    db.init_database_();

    std::string masterPassword;
    std::cout << "Enter master password: ";
    std::getline(std::cin, masterPassword);

    while (true) {
        std::cout << "\n1) Add Entry\n2) Search\n3) Delete Entry\n4) Exit\nChoose: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) {
            std::string title, url, username, password, notes;
            std::cout << "Title: "; std::getline(std::cin, title);
            std::cout << "URL: "; std::getline(std::cin, url);
            std::cout << "Username: "; std::getline(std::cin, username);
            std::cout << "Password: "; std::getline(std::cin, password);
            std::cout << "Notes: "; std::getline(std::cin, notes);
            db.add_entry_(title, url, username, password, notes, masterPassword);
        } else if (choice == 2) {
            std::string query;
            std::cout << "Search query: ";
            std::getline(std::cin, query);
            auto results = db.search_entries_(query, masterPassword);
            for (auto& entry : results) {
                std::cout << "Title: " << entry.m_title << ", Username: " << entry.m_username << std::endl;
            }
        } else if (choice == 4) break;
    }
}

