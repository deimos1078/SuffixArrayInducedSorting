#include <string>
#include <iostream>
#include <limits>

#include "SuffixArray.h"

int main () {
    std::string userInput;
    std::cout << "Enter 'exit' to quit or input string to test suffix array (use quotes for strings with spaces): ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer
    while (std::getline(std::cin, userInput) && userInput != "exit") {
        if (userInput.empty()) {
            continue; // If the user just presses enter, ask for input again.
        }
        // Remove the quotes if user puts them
        if (userInput.size() >= 2 && userInput.front() == '"' && userInput.back() == '"') {
            userInput = userInput.substr(1, userInput.size() - 2);
        }

        SuffixArray suffixArray(userInput);

        suffixArray.printTable();

        std::string pattern;
        std::cout << "Enter pattern to search (use quotes for patterns with spaces): ";
        while (std::getline(std::cin, pattern) && pattern != "exit") {
            if (pattern.empty()) {
                continue; // If the user just presses enter, ask for input again.
            }
            // Remove the quotes if user puts them
            if (pattern.size() >= 2 && pattern.front() == '"' && pattern.back() == '"') {
                pattern = pattern.substr(1, pattern.size() - 2);
            }
            
            std::vector<int> results = suffixArray.search(pattern);

            std::cout << "Results for '" << pattern << "':";
            for (int index : results) {
                std::cout << " " << index;
            }
            std::cout << std::endl;

            std::cout << "Enter next pattern to search or 'exit' to enter new string: ";
        }

        if (pattern == "exit") {
            std::cout << "Enter 'exit' to quit or input string to test suffix array (use quotes for strings with spaces): ";
        }
    }
}