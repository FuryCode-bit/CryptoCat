#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "utils.h"
#include "crypto.h"

/**
 * @brief Main function for generating a single rainbow chain.
 *
 * This program generates a single rainbow chain starting from a given password,
 * with a specified password length and chain length. The generated chain pair
 * (start password and end password) is then written to an output file.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 *             Expected arguments: <start_password> <password_len> <chain_len_k> <outfile>
 * @return 0 if successful, 1 if an error occurs.
 */

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <start_password> <password_len> <chain_len_k> <outfile>" << std::endl;
        return 1;
    }

    try {
        std::string start_pass = argv[1];
        int l = std::stoi(argv[2]);
        uint32_t k = std::stoul(argv[3]);
        std::string filename = argv[4];
        
        if (start_pass.length() != l) {
            std::cerr << "Error: Start password length must match password_len." << std::endl;
            return 1;
        }

        std::cout << "Generating a single, specific chain..." << std::endl;
        std::cout << "  Start Pass: " << start_pass << std::endl;
        std::cout << "  Chain Len (k): " << k << std::endl;

        long dummy_aes_count = 0;
        std::string current_pass = start_pass;

        for (uint32_t j = 0; j < k; ++j) {
            std::vector<unsigned char> hash = hash_password(current_pass, dummy_aes_count);
            current_pass = reduce(hash, j, l);
        }
        std::string end_pass = current_pass;

        std::cout << "  End Pass:   " << end_pass << std::endl;

        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }
        
        outfile.write(reinterpret_cast<const char*>(&l), sizeof(l));
        outfile.write(reinterpret_cast<const char*>(&k), sizeof(k));
        outfile.write(end_pass.c_str(), l);
        outfile.write(start_pass.c_str(), l);
        outfile.close();
        std::cout << "Wrote perfect 1-chain table to " << filename << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}