#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>

#include "../src/SuffixArray.h"

struct TestDataSet {
    std::string testString;
    std::vector<size_t> expectedSuffixArray;
    std::set<std::pair<std::string, std::vector<size_t>>> patternSearchTests; // Pair of pattern and expected indexes
};

int main()
{
    std::cout << "Running basic tests..." << std::endl;

    std::vector<TestDataSet> testData = {
        {
            "mmiissiissiippii", 
            {16, 15, 14, 10, 6, 2, 11, 7, 3, 1, 0, 13, 12, 9, 5, 8, 4}, 
            {{"iss", {3, 7}}, {"s", {4, 5, 8, 9}}, {"ss", {4, 8}}}},
        {
            "swiss_miss",
            {10,5,7,2,6,9,4,8,3,0,1},
            {{"iss", {2,7}}, {"s", {0, 3, 4, 8, 9}}, {"ss", {3, 8}}}
        },
        {
            "abaabababbabbb",
            {14,2,0,3,5,7,10,13,1,4,6,9,12,8,11},
            {{"abaa", {0}}, {"bab", {4, 6, 9}}, {"aba", {0, 3, 5}}}
        },
        {
            "a",
            {1, 0},
            {{"a", {0}}, {"b", {}}}
        },
        {
            "aaaa",
            {4, 3, 2, 1, 0},
            {{"a", {0, 1, 2, 3}}, {"aa", {0, 1, 2}}, {"aaa", {0, 1}}, {"aaaa", {0}}}
        },
        {
            "abcd",
            {4, 0, 1, 2, 3},
            {{"a", {0}}, {"d", {3}}, {"ab", {0}}, {"cd", {2}}}
        },
        {
            "ababababab",
            {10, 8, 6, 4, 2, 0, 9, 7, 5, 3, 1},
            {{"ab", {0, 2, 4, 6, 8}}, {"aba", {0, 2, 4, 6}}, {"abab", {0, 2, 4, 6}}}
        },
        {
            "racecar",
            {7, 1, 5, 4, 2, 3, 6, 0},
            {{"race", {0}}, {"car", {4}}, {"ace", {1}}, {"a", {1, 5}}}
        }
    };

    for (const auto& dataSet : testData) {
        std::cout << "Test String: " << dataSet.testString << std::endl;

        SuffixArray suffixArray(dataSet.testString);

        std::cout << "Test String: Success:";
        assert(suffixArray.getSA() == dataSet.expectedSuffixArray);
        // Check pattern searches
        for (const auto& patternTest : dataSet.patternSearchTests) {
            std::cout << "Looking for " << patternTest.first << std::endl;
            std::vector<size_t> actualResults = suffixArray.search(patternTest.first);

            // Sort both actual results and expected results before comparison
            std::sort(actualResults.begin(), actualResults.end());
            std::vector<size_t> expectedResults = patternTest.second;
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

    return 0;
}