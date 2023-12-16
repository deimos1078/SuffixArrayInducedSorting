#include "SuffixArray.h"
#include <climits>
#include <algorithm>
#include <iostream>

    
// Constructor for the SuffixArray class. Initializes and builds the suffix array for the given input string.
SuffixArray::SuffixArray(const std::string& input_string) {
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
SuffixArray::SuffixArray(const std::vector<int>& S0) {
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

std::vector<int> SuffixArray::search(const std::string &pattern) {
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
std::string SuffixArray::getTypeArrayString() {
    std::string t;
    int tLen = typeTArray.size();
    t.resize(tLen);
    for (int i = 0; i < tLen; i++){
        t[i] = typeTArray[i] ? 'S' : 'L';
    }
    return t;
}

// functions that allow external validation of these array (tests)
std::vector<int> SuffixArray::getSamplePointerArray() {return samplePointerArray;}
std::vector<int> SuffixArray::getSA() {return SA;}

// To visualize the suffix array construction
void SuffixArray::printTable() {
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


// Convert string to vector of ints and append sentinel
void SuffixArray::constructS(const std::string &str) {  
    S.reserve(str.length() + 1);

    std::transform(str.begin(), str.end(), std::back_inserter(S), [](char c) {
        return static_cast<int>(c - 'A');
    });

    S.push_back(INT_MIN); // Append sentinel
}


// Two functions used to construct the buckets
void SuffixArray::calcCharCounts(){
    for (int c : S) 
        charCounts[c]++;
}
void SuffixArray::initBuckets(){
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
void SuffixArray::constructTTypeArray() {    
    typeTArray.resize(S.size());

    // Initialize the last character as S-type
    typeTArray[S.size() - 1] = true;

    // Right-to-left iteration
    for (int i = S.size() - 2; i >= 0; i--) {
        typeTArray[i] = (S[i] < S[i + 1]) || (S[i] == S[i + 1] && typeTArray[i + 1]);
    }
}

void SuffixArray::constructSamplePointerArray() {
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


int SuffixArray::getLMSSubstrLen(size_t SPAIndex){
    if (SPAIndex == samplePointerArray.size() - 1)
        return S.size() - samplePointerArray[SPAIndex];
    return samplePointerArray[SPAIndex + 1] - samplePointerArray[SPAIndex] + 1;
}

bool SuffixArray::constructS1AndCheckAllUniqueLetters() {
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

std::vector<int> SuffixArray::findAllOccurances(int suffixArrayMatchIndex, int patternLength) {
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

std::vector<int> SuffixArray::getSA1(){return S1;};

void SuffixArray::inducedSort(std::vector<int> arrToLoad, bool LMSInduction) {
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

int SuffixArray::LCPRec(int i, int j){
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

void SuffixArray::constructLCPArray() {
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

int SuffixArray::getLCP(int index1, int index2) {
    if (index1 + 1 == index2)
        return LCP[index2];
    else
        return LCP[S.size() + ((index1 + index2) / 2)];
}
int SuffixArray::countMatches(std::vector<int> pattern, int startIndex, int patternLength, int stringIndex) {
    int matches = startIndex;
    for (;matches < patternLength; matches++) {
        if (S[stringIndex + matches] != pattern[matches])
            break;
    }
    return matches;
}

std::vector<int> SuffixArray::searchPrivate (std::vector<int> pattern) {
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