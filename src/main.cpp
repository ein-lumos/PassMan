#include "interface/tui.h"
#include "core/database.h"
#include <iostream>

int main() {
    std::cout << "Initializing database...\n";
    Database::initDatabase();

    std::string masterPassword;
    std::cout << "Enter Master Password: ";
    std::getline(std::cin, masterPassword);

    startTUI(masterPassword);
    return 0;
}

