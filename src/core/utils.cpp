#include "utils.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string expand_password(const std::string& password) {
    // Expands a password string to exactly 16 characters by repeating and/or truncating it.

    std::string expanded = password;
    while (expanded.length() < 16) {
        expanded += password;
    }
    return expanded.substr(0, 16);
}

std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    // Converts a vector of unsigned characters (bytes) into a space-separated hexadecimal string.

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char b : bytes) {
        ss << std::setw(2) << static_cast<int>(b) << " ";
    }
    std::string result = ss.str();
    if (!result.empty()) {
        result.pop_back();
    }
    return result;
}

std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    // Converts a hexadecimal string (with or without spaces) into a vector of unsigned characters (bytes).

    std::vector<unsigned char> bytes;
    std::string current_byte_str;
    for (char c : hex) {
        if (isspace(c)) {
            continue;
        }
        current_byte_str += c;
        if (current_byte_str.length() == 2) {
            bytes.push_back(static_cast<unsigned char>(std::stoul(current_byte_str, nullptr, 16)));
            current_byte_str.clear();
        }
    }
    return bytes;
}