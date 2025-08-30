#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <random>
#include <sstream>
#include <chrono> 
#include <iomanip>

#include "utils.h"
#include "crypto.h"

thread_local std::mt19937 gen(std::random_device{}());

// --- Helper to format duration into a human-readable string ---
std::string format_duration(std::chrono::steady_clock::duration d) {
    // Formats a std::chrono::duration into a human-readable string (e.g., "1.5s", "2m 30s", "3h 15m").

    auto total_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
    if (total_seconds < 0.1) return "< 0.1s";

    if (total_seconds < 60.0) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << total_seconds << "s";
        return ss.str();
    }
    
    long long minutes = static_cast<long long>(total_seconds / 60);
    total_seconds -= minutes * 60;
    long long seconds = static_cast<long long>(total_seconds);
    
    if (minutes < 60) {
        std::stringstream ss;
        ss << minutes << "m " << seconds << "s";
        return ss.str();
    }

    long long hours = minutes / 60;
    minutes %= 60;
    
    std::stringstream ss;
    ss << hours << "h " << minutes << "m";
    return ss.str();
}


std::string random_password(int len) {
    // Generates a random password string of a specified length using the defined CHARSET.

    std::uniform_int_distribution<> distrib(0, CHARSET.length() - 1);
    std::string p;
    p.reserve(len);
    for (int i = 0; i < len; ++i) {
        p += CHARSET[distrib(gen)];
    }
    return p;
}

void generate_and_write_chunk(
    const std::string& temp_filename,
    size_t num_chains_to_generate,
    int password_len,
    uint32_t chain_len)
{
    // Generates a chunk of rainbow table chains and writes them to a temporary binary file.

    std::vector<std::pair<std::string, std::string>> output_chunk;
    output_chunk.reserve(num_chains_to_generate);
    long dummy_aes_count = 0;

    for (size_t i = 0; i < num_chains_to_generate; ++i) {
        std::string start_pass = random_password(password_len);
        std::string current_pass = start_pass;
        for (uint32_t j = 0; j < chain_len; ++j) {
            std::vector<unsigned char> hash = hash_password(current_pass, dummy_aes_count);
            current_pass = reduce(hash, j, password_len);
        }
        output_chunk.push_back({start_pass, current_pass});
    }
    
    std::ofstream temp_outfile(temp_filename, std::ios::binary);
    if (!temp_outfile) {
        std::cerr << "Error: Thread could not open temp file " << temp_filename << std::endl;
        return;
    }
    for (const auto& pair : output_chunk) {
        temp_outfile.write(pair.second.c_str(), password_len);
        temp_outfile.write(pair.first.c_str(), password_len);
    }
}

int main(int argc, char* argv[]) {
    // Main function for the rainbow table generation tool, handling arguments and parallel chain creation.

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <password_length> <num_chains> <output_file.dat> [--threads N] [--verbose]" << std::endl;
        return 1;
    }

    auto total_start = std::chrono::steady_clock::now();

    try {
        int l = std::stoi(argv[1]);
        uint64_t m = std::stoull(argv[2]);
        std::string filename = argv[3];
        int num_threads = 1;
        bool verbose = false;

        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--threads" && i + 1 < argc) {
                num_threads = std::stoi(argv[++i]);
                if (num_threads <= 0) num_threads = 1;
            } else if (arg == "--verbose") {
                verbose = true;
            }
        }

        uint64_t total_passwords_N = static_cast<uint64_t>(pow(CHARSET.length(), l));
        uint32_t k = (m > 0) ? (total_passwords_N / m) : 0;
        if (m == 0 || k == 0) throw std::runtime_error("Calculated table parameters are invalid.");

        if (verbose) {
            std::cout << "--- Generating Rainbow Table (Parallel I/O) ---" << std::endl;
            std::cout << "  Using " << num_threads << " thread(s)." << std::endl;
        }
        
        auto generation_start = std::chrono::steady_clock::now();
        std::vector<std::thread> threads;
        std::vector<std::string> temp_files;
        size_t chains_per_thread = m / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            std::stringstream ss;
            ss << filename << ".part" << i;
            temp_files.push_back(ss.str());
            size_t chains_for_this = (i == num_threads - 1) ? m - (i * chains_per_thread) : chains_per_thread;
            threads.emplace_back(generate_and_write_chunk, temp_files.back(), chains_for_this, l, k);
        }
        for (auto& t : threads) {
            t.join();
        }
        auto generation_end = std::chrono::steady_clock::now();
        if (verbose) {
            std::cout << "  Chain generation and writing took: " << format_duration(generation_end - generation_start) << std::endl;
            std::cout << "Consolidating temporary files..." << std::endl;
        }
        
        auto consolidation_start = std::chrono::steady_clock::now();
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile) throw std::runtime_error("Could not open final output file.");

        outfile.write(reinterpret_cast<const char*>(&l), sizeof(l));
        outfile.write(reinterpret_cast<const char*>(&k), sizeof(k));
        
        for (const auto& temp_file : temp_files) {
            std::ifstream infile(temp_file, std::ios::binary);
            outfile << infile.rdbuf();
            infile.close();
            remove(temp_file.c_str());
        }
        outfile.close();
        auto consolidation_end = std::chrono::steady_clock::now();

        if (verbose) {
             auto total_end = std::chrono::steady_clock::now();
             std::cout << "  File consolidation took: " << format_duration(consolidation_end - consolidation_start) << std::endl;
             std::cout << "✅ Table generation done. (Total time: " << format_duration(total_end - total_start) << ")" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}