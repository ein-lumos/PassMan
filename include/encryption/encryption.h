#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>
#include <string>

class encryption_t {
public:
    encryption_t();

    std::vector<unsigned char> derive_key_(const std::string& masterPassword);

    std::vector<unsigned char> encrypt_aes_(const std::string& plaintext, const std::vector<unsigned char>& key);

    std::string decrypt_aes_(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key);
};

#endif // ENCRYPTION_H

