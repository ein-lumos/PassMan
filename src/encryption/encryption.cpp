#include "encryption/encryption.h"
#include "config.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>

encryption_t::encryption_t() {}

/**
 * @brief Generates an AES key from a master password using PBKDF2.
 */
std::vector<unsigned char> encryption_t::derive_key_(const std::string& masterPassword) {
    std::vector<unsigned char> key(32); // 16 байт - ключ, 16 байт - IV

    PKCS5_PBKDF2_HMAC_SHA1(
        masterPassword.c_str(), masterPassword.length(),
        c_static_salt.data(), c_static_salt.size(),
        10000, // Количество итераций
        key.size(), key.data()
    );

    return key;
}

/**
 * @brief Encrypts a plaintext password using AES-128-CBC.
 */
std::vector<unsigned char> encryption_t::encrypt_aes_(const std::string& plaintext, const std::vector<unsigned char>& key) {
    std::vector<unsigned char> iv(key.begin() + 16, key.begin() + 32); // IV = последние 16 байт ключа
    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len, ciphertext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data());

    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size());
    ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

/**
 * @brief Decrypts an AES-128-CBC encrypted password.
 */
std::string encryption_t::decrypt_aes_(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key) {
    std::vector<unsigned char> iv(key.begin() + 16, key.begin() + 32); // IV = последние 16 байт ключа
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len, plaintext_len;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv.data());

    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
    plaintext_len = len;

    EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);

    return std::string(plaintext.begin(), plaintext.end());
}

