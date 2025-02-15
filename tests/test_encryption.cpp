#include "encryption/encryption.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

void print_hex(const std::vector<unsigned char>& data) {
    for (unsigned char c : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    std::cout << std::dec << std::endl;
}

std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        bytes.push_back(static_cast<unsigned char>(std::stoi(byteString, nullptr, 16)));
    }
    return bytes;
}

int main() {
    encryption_t encryption;
    std::string masterPassword, input;
    int choice;

    std::cout << "=== Encryption Test ===\n";
    std::cout << "1) Encrypt Text\n";
    std::cout << "2) Decrypt Hex\n";
    std::cout << "Choose option: ";
    std::cin >> choice;
    std::cin.ignore(); // Очистить буфер после ввода числа

    std::cout << "Enter master password: ";
    std::getline(std::cin, masterPassword);

    std::vector<unsigned char> key = encryption.derive_key_(masterPassword);
    std::cout << "Derived Key: ";
    print_hex(key);

    if (choice == 1) {
        std::cout << "Enter text to encrypt: ";
        std::getline(std::cin, input);

        std::vector<unsigned char> ciphertext = encryption.encrypt_aes_(input, key);
        std::cout << "Encrypted text (hex): ";
        print_hex(ciphertext);

    } else if (choice == 2) {
        std::cout << "Enter encrypted hex: ";
        std::getline(std::cin, input);

        std::vector<unsigned char> ciphertext = hex_to_bytes(input);
        std::string decryptedText = encryption.decrypt_aes_(ciphertext, key);

        std::cout << "Decrypted text: " << decryptedText << std::endl;
    } else {
        std::cout << "Invalid option!" << std::endl;
    }

    return 0;
}

