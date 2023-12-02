#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_map>
    
class SuffixArray {
public:
// Constructor for the SuffixArray class. Initializes and builds the suffix array for the given input string.
    SuffixArray(const std::string& input_string) {
        // turn the input string into a vector of ints, which allows for a higher range of characters
        // necessary for the recusrive S1 calling, which creates more unique characters than char can handle
        constructS(input_string);

        // character counts can be directly used to create the bucket array
        calcCharCounts();
        initBuckets();

        // Construct the type array (T-type array) which classifies each character in the input string as either L-type or S-type.
        constructTTypeArray();

        // Use T-type array to Construct the sample pointer array. This array is used in the induced sorting of LMS substrings.
        constructSamplePointerArray();

        // Perform induced sorting on LMS (Left-Most S-type) substrings. .
        inducedSort(samplePointerArray, true);

        // Check if all the characters in the reduced string S1 are unique.
        // If all characters in S1 are unique, directly compute the suffix array SA1.
        // Otherwise, recursively construct the suffix array for the reduced string S1.
        if (constructS1AndCheckAllUniqueLetters() == true){ 
            inducedSort(getSA1(), false);
        } else {
            SuffixArray suffixArray(S1);
            inducedSort(suffixArray.SA, false);
        }

        // Construct the enchanced LCP (Longest Common Prefix) array
        // Used for optimilization of search
        constructLCPArray();
    }

    // overload for accepted vector of ints as an input through recursive calls
    // LCP also is not needed to contruct so it is ommitted
    SuffixArray(const std::vector<int>& S0) {
        S = S0;
        calcCharCounts();
        initBuckets();
        constructTTypeArray();
        constructSamplePointerArray();
        inducedSort(samplePointerArray, true);
        if (constructS1AndCheckAllUniqueLetters() == true){ 
            inducedSort(S1, false);
        } else {
            SuffixArray suffixArray(S1);
            inducedSort(suffixArray.SA, false);
        }
    }

    std::vector<int> search(const std::string &pattern) {
        // Prepare the pattern to be searched by turning it into a vector of ints
        // the same way the string was processed
        if (pattern.empty())
            return {};
        std::vector<int> patternInt;
        patternInt.resize(pattern.length());
        for (size_t i = 0; i < pattern.length(); i++) 
            patternInt[i] = pattern[i] - 'A';

        // search logic is within the private part of the class
        return searchPrivate(patternInt);
}
    
    // To help printing out the t array, which is a vector of bools 
    std::string getTypeArrayString() {
        std::string t;
        int tLen = typeTArray.size();
        t.resize(tLen);
        for (int i = 0; i < tLen; i++){
            t[i] = typeTArray[i] ? 'S' : 'L';
        }
        return t;
    }

    // functions that allow external validation of these array (tests)
    std::vector<int> getSamplePointerArray() {return samplePointerArray;}
    std::vector<int> getSA() {return SA;}
    
    // To visualize the suffix array construction
    void printTable() {
        // print indexes
        std::cout << "i: ";
        for (size_t i = 0; i < S.size(); i++)
            std::cout << i % 10;
        std::cout << std::endl;

        // print string
        std::cout << "S: ";
        for (int i : S) {
            char c;
            if (i == INT_MIN)
                c = '$';
            else
                c = 'A' + i;
            std::cout << c;
        }
        std::cout << std::endl;

        // print t type array
        std::cout << "t: ";
        for (size_t i = 0; i < S.size(); i++) {
            std::cout << (typeTArray[i] ? "S" : "L"); 
        }
        std::cout << std::endl;

        // print lms char pointers
        std::cout << "P: ";
        int curSPAIndex = 0;
        for (int i = 0; i < int(S.size()); i++) {
            if (i == samplePointerArray[curSPAIndex]){
                curSPAIndex ++;
                std::cout << "*";
            } else
                std::cout << " ";
        }
        std::cout << std::endl;

        // print SA
        std::cout << "SA:";
        for (int i : SA){
            std::cout << i << ",";
        }
        std::cout << std::endl;
    };

private:
    // Init necessary variables
    std::vector<bool> typeTArray;
    std::vector<int> samplePointerArray;
    std::vector<int> SA;
    std::vector<int> S;
    std::vector<int> S1;
    std::vector<int> LCP;
    std::vector<int> buckets;
    std::map<int, int> charCounts;
    std::unordered_map<int, int> charToBucket;
    int bucketsLen;

    // Convert string to vector of ints and append sentinel
    void constructS(const std::string &str) {  
        S.reserve(str.length() + 1);

        std::transform(str.begin(), str.end(), std::back_inserter(S), [](char c) {
            return static_cast<int>(c - 'A');
        });

        S.push_back(INT_MIN); // Append sentinel
    }


    // Two functions used to construct the buckets
    void calcCharCounts(){
        for (int c : S) 
            charCounts[c]++;
    }
    void initBuckets(){
        int curIndex = 0;
        bucketsLen = 0;
        for (auto &pair : charCounts) {
            buckets.push_back(curIndex);
            charToBucket[pair.first] = bucketsLen++;
            curIndex += pair.second;
        }
    }

    // true = S-type
    // false = L-type
    void constructTTypeArray() {    
        typeTArray.resize(S.size());

        // Initialize the last character as S-type
        typeTArray[S.size() - 1] = true;

        // Right-to-left iteration
        for (int i = S.size() - 2; i >= 0; i--) {
            typeTArray[i] = (S[i] < S[i + 1]) || (S[i] == S[i + 1] && typeTArray[i + 1]);
        }
    }

    void constructSamplePointerArray() {
        samplePointerArray.reserve(S.size());

        // when first char is S type
        if (typeTArray[0] == true)
            samplePointerArray.push_back(0);

        for (size_t i = 1; i < S.size(); i++) {
            if (typeTArray[i] == true && !typeTArray[i - 1]) { // Current is S-type and previous is L-type
                samplePointerArray.push_back(i);
            }
        }
    }


    int getLMSSubstrLen(size_t SPAIndex){
        if (SPAIndex == samplePointerArray.size() - 1)
            return S.size() - samplePointerArray[SPAIndex];
        return samplePointerArray[SPAIndex + 1] - samplePointerArray[SPAIndex] + 1;
    }

    bool constructS1AndCheckAllUniqueLetters() {
        int sampleSize = samplePointerArray.size();
        S1.resize(sampleSize);

        // Pre-building the map
        std::unordered_map<int, int> SPAReverseMap;
        for (int i = 0; i < sampleSize; i++)
            SPAReverseMap[samplePointerArray[i]] = i;

        bool allUniqueLetters = true;
        int curBucketIndex = -1;
        int prevFoundLMSCharIndex = -1; // Initialize to -1 as sentinel
        int prevSubstrLen = 0;
        int bucketStart, bucketEnd;

        for (int bucketIdx = 0; bucketIdx < bucketsLen; bucketIdx++) {
            bucketStart = buckets[bucketIdx];
            bucketEnd = (bucketIdx + 1 == bucketsLen) ? S.size() - 1 : buckets[bucketIdx + 1] - 1;

            bool foundInBucket = false;
            for (int j = bucketStart; j <= bucketEnd; j++) {
                auto it = SPAReverseMap.find(SA[j]);
                if (it == SPAReverseMap.end()) continue;

                int curSubstrLen = getLMSSubstrLen(it->second);

                if (foundInBucket) {
                    if (curSubstrLen == prevSubstrLen && std::equal(S.begin() + SA[prevFoundLMSCharIndex], S.begin() + SA[prevFoundLMSCharIndex] + curSubstrLen, S.begin() + SA[j])) {
                        allUniqueLetters = false;
                        S1[it->second] = curBucketIndex;
                        continue;
                    }
                }

                prevFoundLMSCharIndex = j;
                prevSubstrLen = curSubstrLen;
                S1[it->second] = ++curBucketIndex;
                foundInBucket = true;
            }
        }

        return allUniqueLetters;
    }

    std::vector<int> findAllOccurances(int suffixArrayMatchIndex, int patternLength) {
        std::vector<int> matches = {SA[suffixArrayMatchIndex]};

        // check to the left of intial match
        for (int suffixArrayIndex = suffixArrayMatchIndex; suffixArrayIndex >= 0; suffixArrayIndex--) {
            if (LCP[suffixArrayIndex] >= patternLength)
                matches.push_back(SA[suffixArrayIndex - 1]);
            else
                break;
        }

        // check to the left of intial match
        for (size_t suffixArrayIndex = suffixArrayMatchIndex + 1; suffixArrayIndex < S.size(); suffixArrayIndex++) {
            if (LCP[suffixArrayIndex] >= patternLength)
                matches.push_back(SA[suffixArrayIndex]);
            else
                break;
        }
        return matches;
    }

    std::vector<int> getSA1(){return S1;};

    void inducedSort(std::vector<int> arrToLoad, bool LMSInduction) {
        // auto-load size 1 buckets
        SA.clear();
        SA.resize(S.size(), -1);

        for (size_t i = 0; i < S.size(); i++)
            if (charCounts[S[i]] == 1)
                SA[buckets[charToBucket[S[i]]]] = i;
        
        std::vector<int> tails;
        tails.resize(bucketsLen);
        for (int i = 1; i < bucketsLen; i++) 
            tails[i - 1] = buckets[i] - 1;
        tails[bucketsLen - 1] = S.size() - 1;

        if (LMSInduction == false) { // arrToLoad = SA1
            for (int i = arrToLoad.size() - 1; i >= 0; i--){
                int indexToLoad = samplePointerArray[arrToLoad[i]];  // get the index that will be inserted into SuffixArray
                SA[tails[charToBucket[S[indexToLoad]]]--] = indexToLoad; // load into tail of bucket, while also moving the tail
            }
        } else { // arrToLoad = samplePointerArray
            for (int i = 0; i < arrToLoad.size(); i++) {
                int indexToLoad = arrToLoad[i]; 
                SA[tails[charToBucket[S[indexToLoad]]]--] = indexToLoad;
            }
        }

        // restore tail indexes
        for (int i = 1; i < bucketsLen; i++) 
            tails[i - 1] = buckets[i] - 1;
        tails[bucketsLen - 1] = S.size() - 1;

        std::vector<int> heads;
        heads.resize(bucketsLen);
        for (int i = 0; i < bucketsLen; i++) 
            heads[i] = buckets[i];


        for (size_t i = 0; i < S.size(); i++) {
            if (SA[i] == -1) continue;
            if (SA[i] == 0) continue;
            if (typeTArray[SA[i] - 1] == false) // if previous suffix is L type
                SA[heads[charToBucket[S[SA[i] - 1]]]++] = SA[i] - 1;
        }

        for (int i = S.size() - 1; i >= 0; i--){
            if (SA[i] == -1) continue;
            if (SA[i] == 0) continue;
            if (typeTArray[SA[i] - 1] == true) // if previous suffix is S type
                SA[tails[charToBucket[S[SA[i] - 1]]]--] = SA[i] - 1;
        }  
        
    }

    int LCPRec(int i, int j){
        int res;
        int mid = (i + j) / 2;
        if (j - i == 2) {
            res = std::min(LCP[i + 1], LCP[j]); // LCP(2, 4) = min(LCP(2, 3), LCP(3, 4)) = LCP[n + 3]
        } else if (j - i == 3) {
            res = std::min(LCP[i + 1], LCPRec(i + 1, j)); // LCP(2, 5) = min(LCP(2,3), LCP(3,5)) = LCP[n + 3]
        } else {
            res = std::min(LCPRec(i, mid), LCPRec(mid, j));
        }
        LCP[S.size() + mid] = res;
        return res;
    }

    void constructLCPArray() {
        std::vector<size_t> rank(S.size(), 0);
        LCP.resize(S.size() * 2 - 1, 0);

        // Building the rank array
        for (size_t i = 0; i < S.size(); i++) {
            rank[SA[i]] = i;
        }

        size_t k = 0;
        for (size_t i = 0; i < S.size(); i++, k ? k-- : 0) {
            if (rank[i] == S.size() - 1) {
                k = 0;
                continue;
            }
            size_t j = SA[rank[i] + 1];
            while (i + k < S.size() && j + k < S.size() && S[i + k] == S[j + k]) {
                k++;
            }
            LCP[rank[i] + 1] = k; // LCP between the current and next suffix in the suffix array
        }
        LCP[0] = 0;
        LCPRec(-1, S.size() - 1);
    }

    int getLCP(int index1, int index2) {
        if (index1 + 1 == index2)
            return LCP[index2];
        else
            return LCP[S.size() + ((index1 + index2) / 2)];
    }
    int countMatches(std::vector<int> pattern, int startIndex, int patternLength, int stringIndex) {
        int matches = startIndex;
        for (;matches < patternLength; matches++) {
            if (S[stringIndex + matches] != pattern[matches])
                break;
        }
        return matches;
    }

    std::vector<int> searchPrivate (std::vector<int> pattern) {
        int patternLength = pattern.size();
        
        int low = -1;
        int lowMatches = 0;
        int high = S.size() - 1;
        int highMatches = countMatches(pattern, 0, patternLength, SA[high]);

        if (highMatches == patternLength)
            return findAllOccurances(high, patternLength);

        int mid;
        int lcpLow, lcpHigh;
        int maxMatches;

        while (low + 1 < high) {
            mid = (low + high) / 2;

            lcpHigh = getLCP(mid, high);
            lcpLow = getLCP(low, mid);

            if (lowMatches <= lcpHigh && lcpHigh < highMatches) {
                // mid does not overlap with high
                // enough to match more of the pattern
                // better match is somewhere between mid and high
                low = mid;
                lowMatches = lcpHigh;
            } else if (lowMatches <= highMatches && highMatches < lcpHigh) {
                // mid overlaps with high more than high with pattern
                // pattern matches the mid the same as it does high
                high = mid;
            // these two are analogous
            } else if (highMatches <= lcpLow && lcpLow < lowMatches) {
                high = mid;
                highMatches = lcpLow;
            } else if (highMatches <= lowMatches && lowMatches < lcpLow) {
                low = mid;
            } else {
                // If we are here, we could not find a reason to
                // not to compare mid to pattern
                maxMatches = std::max(lowMatches, highMatches);
                maxMatches = countMatches(pattern, maxMatches, patternLength, SA[mid]);

                if (maxMatches == patternLength) // if exact match occurs
                    return findAllOccurances(mid, patternLength);
                else if (S[SA[mid] + maxMatches] < pattern[maxMatches]) { // unmatched letter of pattern is bigger
                    low = mid;
                    lowMatches = maxMatches;
                } else {
                    high = mid;
                    highMatches = maxMatches;
                }
            }
        }
        return {};
    }
};

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

// Function to read FASTA file and return a sequence
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
