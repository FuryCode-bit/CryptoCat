#include "crypto.h"
#include "utils.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdexcept>

void handle_openssl_errors() {
    // Throws a runtime error indicating a failure in an OpenSSL crypto library call.
    throw std::runtime_error("A call to OpenSSL's crypto library failed.");
}

std::vector<unsigned char> aes128_encrypt(const std::string& plaintext_str, const std::string& key_str) {
    // Encrypts a 16-byte plaintext using AES-128 in ECB mode with a 16-byte key and no padding.
    
    const unsigned char* plaintext = reinterpret_cast<const unsigned char*>(plaintext_str.c_str());
    const unsigned char* key = reinterpret_cast<const unsigned char*>(key_str.c_str());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_errors();

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    std::vector<unsigned char> ciphertext(16 + EVP_CIPHER_block_size(EVP_aes_128_ecb()));
    int len;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext, 16)) {
        EVP_CIPHER_CTX_free(ctx);
        handle_openssl_errors();
    }
    ciphertext.resize(len);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

std::vector<unsigned char> hash_password(const std::string& password, long& aes_eval_count) {
    // Hashes a password by expanding it and then AES-128 encrypting it with itself as the key.

    std::string key_and_data = expand_password(password);
    aes_eval_count++;
    return aes128_encrypt(key_and_data, key_and_data);
}

std::string reduce(const std::vector<unsigned char>& hash, int position, int password_len) {
    // Reduces a hash back into a password string of a specified length using the given position.

    uint64_t hash_val = 0;
    for (int i = 0; i < 8 && i < hash.size(); ++i) {
        hash_val = (hash_val << 8) | hash[i];
    }
    hash_val += position;

    std::string next_password = "";
    uint64_t temp_val = hash_val;
    for (int i = 0; i < password_len; ++i) {
        next_password += CHARSET[temp_val % CHARSET.length()];
        temp_val /= CHARSET.length();
    }
    return next_password;
}