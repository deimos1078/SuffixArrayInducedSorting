#include "SuffixArray.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <sstream>
#include <chrono>
#include <set>
#include <algorithm>

struct TestDataSet {
    std::string testString;
    std::string expectedTTypeArray;
    std::vector<int> expectedSamplePointerArray;
    std::vector<int> expectedSuffixArray;
    std::set<std::pair<std::string, std::vector<int>>> patternSearchTests; // Pair of pattern and expected indexes
};

std::vector<int> bruteForceSearch (const std::string &string, const std::string &pattern) {
    std::vector<int> matchIndexes;

    int stringLength = string.length();
    int patternLength = pattern.length();

    for (int i = 0; i < stringLength - patternLength; i++) {
        for (int j = 0; j <= patternLength; j++) {
            if (j == patternLength) {
                matchIndexes.push_back(i);
                break;
            }
            if (string[i + j] != pattern[j] )
                break;
        }   
    }
    return matchIndexes;
}

// Function to read file and return a sequence
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

int main()
{
    std::cout << "Running basic tests..." << std::endl;

    std::vector<TestDataSet> testData = {
        {
            "mmiissiissiippii", 
            "LLSSLLSSLLSSLLLLS", 
            {2, 6, 10, 16}, 
            {16, 15, 14, 10, 6, 2, 11, 7, 3, 1, 0, 13, 12, 9, 5, 8, 4}, 
            {{"iss", {3, 7}}, {"s", {4, 5, 8, 9}}, {"ss", {4, 8}}}},
        {
            "swiss_miss",
            "SLSLLSLSLLS",
            {0,2,5,7,10},
            {10,5,7,2,6,9,4,8,3,0,1},
            {{"iss", {2,7}}, {"s", {0, 3, 4, 8, 9}}, {"ss", {3, 8}}}
        },
        {
            "abaabababbabbb",
            "SLSSLSLSLLSLLLS",
            {0,2,5,7,10,14},
            {14,2,0,3,5,7,10,13,1,4,6,9,12,8,11},
            {{"abaa", {0}}, {"bab", {4, 6, 9}}, {"aba", {0, 3, 5}}}
        },
        {
            "a",
            "LS",
            {1},
            {1, 0},
            {{"a", {0}}, {"b", {}}}
        },
        {
            "aaaa",
            "LLLLS",
            {4},
            {4, 3, 2, 1, 0},
            {{"a", {0, 1, 2, 3}}, {"aa", {0, 1, 2}}, {"aaa", {0, 1}}, {"aaaa", {0}}}
        },
        {
            "abcd",
            "SSSLS",
            {0, 4},
            {4, 0, 1, 2, 3},
            {{"a", {0}}, {"d", {3}}, {"ab", {0}}, {"cd", {2}}}
        },
        {
            "ababababab",
            "SLSLSLSLSLS",
            {0, 2, 4, 6, 8, 10},
            {10, 8, 6, 4, 2, 0, 9, 7, 5, 3, 1},
            {{"ab", {0, 2, 4, 6, 8}}, {"aba", {0, 2, 4, 6}}, {"abab", {0, 2, 4, 6}}}
        },
        {
            "racecar",
            "LSSLLSLS",
            {1, 5, 7},
            {7, 1, 5, 4, 2, 3, 6, 0},
            {{"race", {0}}, {"car", {4}}, {"ace", {1}}, {"a", {1, 5}}}
        }
    };

    for (const auto& dataSet : testData) {
        std::cout << "Test String: " << dataSet.testString << std::endl;

        SuffixArray suffixArray(dataSet.testString);

        suffixArray.printTable();
        // Check T-type array
        assert(suffixArray.getTypeArrayString() == dataSet.expectedTTypeArray);

        // Check Sample Pointer Array
        assert(suffixArray.getSamplePointerArray() == dataSet.expectedSamplePointerArray);

        // Check Suffix Array - if you have expected values for this
        // assert(suffixArray.getSA() == dataSet.expectedSuffixArray);

        // Check pattern searches
        for (const auto& patternTest : dataSet.patternSearchTests) {
            std::cout << "Looking for " << patternTest.first << std::endl;
            std::vector<int> actualResults = suffixArray.search(patternTest.first);

            // Sort both actual results and expected results before comparison
            std::sort(actualResults.begin(), actualResults.end());
            std::vector<int> expectedResults = patternTest.second;
            std::sort(expectedResults.begin(), expectedResults.end());

            std::cout << "Found :";
            for (auto& actualResult : actualResults) {
                std::cout << actualResult << ",";
            }
            std::cout << std::endl;

            std::cout << "Expected :";
            for (auto& expectedResult : expectedResults) {
                std::cout << expectedResult << ",";
            }
            std::cout << std::endl;

            assert(actualResults == expectedResults);
        }

        std::cout << "Test passed!" << std::endl;
        std::cout << "----------------------" << std::endl;
    }


    char runTimeTests;
    std::cout << "Do you want to run time tests? (y/n): ";
    std::cin >> runTimeTests;

    if (runTimeTests == 'y' || runTimeTests == 'Y') {
        std::cout << "Running time tests..." << std::endl;
        
        std::vector<std::string> stringFiles = {"string1000.txt", "string5000.txt","string10000.txt", "string50000.txt", 
        "string100000.txt", "string500000.txt", "string1000000.txt"};
        std::vector<std::string> patternFiles = {"patterns100.txt", "patterns200.txt", "patterns300.txt", "patterns400.txt", "patterns500.txt"};

        for (const auto& stringFile : stringFiles) {
            std::string text = readString(stringFile);
            
            auto start = std::chrono::high_resolution_clock::now();
            SuffixArray suffixArray(text);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Suffix Array Creation time for " << stringFile << ": " << elapsed_seconds.count() << "s\n";

            for (const auto& patternFile : patternFiles) {
                std::vector<std::string> patterns = readPatterns(patternFile);
                
                start = std::chrono::high_resolution_clock::now();
                for (const auto& pattern : patterns)
                    suffixArray.search(pattern);
                end = std::chrono::high_resolution_clock::now();
                elapsed_seconds = end - start;
                std::cout << "Suffix Array Search time for " << patternFile << ": " << elapsed_seconds.count() << "s\n";

                start = std::chrono::high_resolution_clock::now();
                for (const auto& pattern : patterns)
                    bruteForceSearch(text, pattern);
                end = std::chrono::high_resolution_clock::now();
                elapsed_seconds = end - start;
                std::cout << "Brute Force Search time for " << patternFile << ": " << elapsed_seconds.count() << "s\n";
            }
            std::cout << "----------------------" << std::endl;

        }
    } else {
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

    return 0;
}