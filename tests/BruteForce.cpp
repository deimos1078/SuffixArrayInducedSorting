#include "BruteForce.h"

BruteForce::BruteForce(const std::string &inputString) : string(inputString) {}

std::vector<int> BruteForce::search(const std::string &pattern) const {
    std::vector<int> matchIndexes;

    for (size_t i = 0; i + pattern.length() <= this->string.length(); ++i) {
        size_t j = 0;
        for (; j < pattern.length(); ++j) {
            if (this->string[i + j] != pattern[j])
                break;
        }
        if (j == pattern.length()) {
            matchIndexes.push_back(i);
        }
    }
    return matchIndexes;
}