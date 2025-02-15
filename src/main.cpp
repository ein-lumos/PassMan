#include "database/database.h"
#include "interface/tui.h"
#include <iostream>

int main() {
    database_t db;
    db.init_database_();

    std::string masterPassword;
    std::cout << "Enter Master Password: ";
    std::getline(std::cin, masterPassword);

    // Запускаем TUI
    start_tui(db, masterPassword);

    return 0;
}

