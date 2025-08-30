#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>

#include "utils.h"
#include "crypto.h"

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

// --- Multi-threaded shared resources ---
std::atomic<bool> g_password_found(false);
std::string g_found_password;
std::atomic<long> g_total_aes_evals(0);
std::mutex g_result_mutex;

// --- Multi-threaded worker ---
void search_chunk(const std::unordered_map<std::string, std::string>& map_chunk, const std::vector<unsigned char>& target_hash, int l, uint32_t k) {
    // Searches a portion of the rainbow table (map_chunk) for a target hash in a multi-threaded context.

    long local_aes_count = 0;
    for (int pos = k - 1; pos >= 0; --pos) {
        if (g_password_found) break;
        std::string candidate_pass;
        std::vector<unsigned char> temp_hash = target_hash;
        for (int j = pos; j < k; ++j) {
            candidate_pass = reduce(temp_hash, j, l);
            if (j < k - 1) temp_hash = hash_password(candidate_pass, local_aes_count);
        }
        auto it = map_chunk.find(candidate_pass);
        if (it != map_chunk.end()) {
            std::string current_pass = it->second;
            for (uint32_t j = 0; j < k; ++j) {
                if (g_password_found) break;
                std::vector<unsigned char> h = hash_password(current_pass, local_aes_count);
                if (h == target_hash) {
                    std::lock_guard<std::mutex> lock(g_result_mutex);
                    if (!g_password_found) {
                        g_password_found = true;
                        g_found_password = current_pass;
                    }
                    break;
                }
                current_pass = reduce(h, j, l);
            }
        }
    }
    g_total_aes_evals += local_aes_count;
}

// --- Single-threaded worker (no atomics/mutexes) ---
std::string search_single_threaded(const std::unordered_map<std::string, std::string>& rainbow_map, const std::vector<unsigned char>& target_hash, int l, uint32_t k, long& aes_count) {
    // Searches the entire rainbow table for a target hash in a single-threaded context.

    for (int pos = k - 1; pos >= 0; --pos) {
        std::string candidate_pass;
        std::vector<unsigned char> temp_hash = target_hash;
        for (int j = pos; j < k; ++j) {
            candidate_pass = reduce(temp_hash, j, l);
            if (j < k - 1) temp_hash = hash_password(candidate_pass, aes_count);
        }
        auto it = rainbow_map.find(candidate_pass);
        if (it != rainbow_map.end()) {
            std::string current_pass = it->second;
            for (uint32_t j = 0; j < k; ++j) {
                std::vector<unsigned char> h = hash_password(current_pass, aes_count);
                if (h == target_hash) {
                    return current_pass;
                }
                current_pass = reduce(h, j, l);
            }
        }
    }
    return "";
}


int main(int argc, char* argv[]) {
    // Main function for the rainbow table cracking tool, handling file loading and search execution.
    
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <rainbow_file> <hash_hex> [--threads N] [--verbose]" << std::endl;
        return 1;
    }
    
    auto total_start = std::chrono::steady_clock::now();

    try {
        std::string filename = argv[1];
        std::string target_hash_hex = argv[2];
        int num_threads = 1;
        bool verbose = false;

        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--threads" && i + 1 < argc) {
                num_threads = std::stoi(argv[++i]);
                if (num_threads <= 0) num_threads = 1;
            } else if (arg == "--verbose") {
                verbose = true;
            }
        }
        
        std::vector<unsigned char> target_hash = hex_to_bytes(target_hash_hex);
        if (target_hash.size() != 16) throw std::runtime_error("Invalid hash length.");

        auto load_start = std::chrono::steady_clock::now();
        std::ifstream infile(filename, std::ios::binary);
        if (!infile) throw std::runtime_error("Could not open rainbow table file.");

        int l;
        uint32_t k;
        infile.read(reinterpret_cast<char*>(&l), sizeof(l));
        infile.read(reinterpret_cast<char*>(&k), sizeof(k));
        if (!infile) throw std::runtime_error("Could not read table parameters.");
        
        bool success = false;
        std::string final_password = "";
        long final_aes_count = 0;
        
        auto search_start = std::chrono::steady_clock::now();

        if (num_threads == 1) {
            std::unordered_map<std::string, std::string> rainbow_map;
            std::vector<char> pass_buffer(l + l);
            while (infile.read(pass_buffer.data(), l + l)) {
                 rainbow_map[std::string(pass_buffer.data(), l)] = std::string(pass_buffer.data() + l, l);
            }
            infile.close();
            auto load_end = std::chrono::steady_clock::now();
            if (verbose) std::cout << "Table loaded with " << rainbow_map.size() << " chains. (took " << format_duration(load_end - load_start) << ")" << std::endl;
            
            search_start = std::chrono::steady_clock::now();
            final_password = search_single_threaded(rainbow_map, target_hash, l, k, final_aes_count);
            if (!final_password.empty()) success = true;

        } else {
            std::vector<std::unordered_map<std::string, std::string>> table_chunks(num_threads);
            std::vector<char> pass_buffer(l + l);
            int current_chunk = 0;
            while (infile.read(pass_buffer.data(), l + l)) {
                 table_chunks[current_chunk][std::string(pass_buffer.data(), l)] = std::string(pass_buffer.data() + l, l);
                 current_chunk = (current_chunk + 1) % num_threads;
            }
            infile.close();
            auto load_end = std::chrono::steady_clock::now();
            if (verbose) std::cout << "Table loaded and split for " << num_threads << " threads. (took " << format_duration(load_end - load_start) << ")" << std::endl;

            search_start = std::chrono::steady_clock::now();
            std::vector<std::thread> threads;
            for (int i = 0; i < num_threads; ++i) {
                threads.emplace_back(search_chunk, std::ref(table_chunks[i]), std::ref(target_hash), l, k);
            }
            for (auto& t : threads) {
                t.join();
            }
            success = g_password_found;
            final_password = g_found_password;
            final_aes_count = g_total_aes_evals;
        }
        auto search_end = std::chrono::steady_clock::now();

        if (success) {
            std::cout << "\n--- SUCCESS ---" << std::endl;
            std::cout << "Password found: " << final_password << std::endl;
        } else {
            std::cout << "\n--- FAILURE ---" << std::endl;
            std::cout << "Password not found in the table." << std::endl;
        }
        if (verbose) {
            auto total_end = std::chrono::steady_clock::now();
            std::cout << "Password search took: " << format_duration(search_end - search_start) << std::endl;
            std::cout << "Total AES evaluations: " << final_aes_count << std::endl;
            std::cout << "Total execution time: " << format_duration(total_end - total_start) << std::endl;
        }
        return success ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}