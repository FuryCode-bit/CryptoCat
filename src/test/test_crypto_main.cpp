#include <iostream>
#include <string>
#include <vector>
#include "utils.h"
#include "crypto.h"

/**
 * @brief Main function for testing password hashing and reduction.
 *
 * This program takes a password as a command-line argument, expands it,
 * hashes it, and then prints the original password, expanded key, and
 * the hexadecimal representation of the hash.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 *             Expected argument: <password_to_test>
 * @return 0 if successful, 1 if an error occurs.
 */
 
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <password_to_test>" << std::endl;
        return 1;
    }
    
    std::string password = argv[1];
    long dummy_aes_count = 0;

    std::cout << "Password:  " << password << std::endl;
    std::cout << "Key:       " << expand_password(password) << std::endl;
    
    std::vector<unsigned char> hash = hash_password(password, dummy_aes_count);
    
    std::cout << "Hash:      " << bytes_to_hex(hash) << std::endl;

    return 0;
}