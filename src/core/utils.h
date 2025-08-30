#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cstdint>

// The 64-character set for passwords
const std::string CHARSET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?!";

// Expands a password to 16 bytes for the AES key
std::string expand_password(const std::string& password);

// Converts a vector of bytes to a hexadecimal string
std::string bytes_to_hex(const std::vector<unsigned char>& bytes);

// Converts a hexadecimal string to a vector of bytes
std::vector<unsigned char> hex_to_bytes(const std::string& hex);

#endif