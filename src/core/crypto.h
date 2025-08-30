#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>

std::vector<unsigned char> hash_password(const std::string& password, long& aes_eval_count);
std::string reduce(const std::vector<unsigned char>& hash, int position, int password_len);

#endif