#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>

#include "../src/SuffixArray.h"
#include "BruteForce.h"

std::string readString(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::string string;
    while (getline(file, line)) {
        string += line;
    }
    return string;
}

std::vector<std::string> readPatterns (const std::string& filename){
    std::ifstream file(filename);
    std::string line;
    std::vector<std::string> patterns;
    while (getline(file, line)) {
        patterns.push_back(line);
    }
    return patterns;
}

int main() {
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    char runTimeTests;

    std::cout << "Running time tests..." << std::endl;
    
    std::vector<std::string> stringFiles = {"string1000000.txt", 
    "string2500000.txt", "string5000000.txt", "string10000000.txt"};
    std::vector<std::string> patternFiles = {"patterns10.txt", "patterns100.txt", "patterns1000.txt", "patterns10000.txt", "patterns100000.txt"};
    size_t found;

    for (const auto& stringFile : stringFiles) {
        std::string text = readString("../data/" + stringFile);
        
        auto start = std::chrono::high_resolution_clock::now();
        SuffixArray suffixArray(text);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Suffix Array Creation time for " << stringFile << ": " << elapsed_seconds.count() << "s\n";

        BruteForce bruteForce(text);

        for (const auto& patternFile : patternFiles) {
            std::vector<std::string> patterns = readPatterns("../data/" + patternFile);
            found = 0;
            start = std::chrono::high_resolution_clock::now();
            for (const auto& pattern : patterns)
                found += suffixArray.search(pattern).size();
            end = std::chrono::high_resolution_clock::now();
            elapsed_seconds = end - start;
            std::cout << "Suffix Array Search time for " << patternFile << ": " << elapsed_seconds.count() << "s\n";
            std::cout << "Found: " << found << " patterns." << std::endl;

            found = 0;
            start = std::chrono::high_resolution_clock::now();
            for (const auto& pattern : patterns)
                found += bruteForce.search(pattern).size();
            end = std::chrono::high_resolution_clock::now();
            elapsed_seconds = end - start;
            std::cout << "Brute Force Search time for " << patternFile << ": " << elapsed_seconds.count() << "s\n";
            std::cout << "Found: " << found << " patterns." << std::endl;
        }
        std::cout << "----------------------" << std::endl;
    }

    return 0;
}