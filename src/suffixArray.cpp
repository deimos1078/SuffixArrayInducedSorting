#include "SuffixArray.h"
#include <climits>
#include <algorithm>
#include <iostream>

    
// Constructor for the SuffixArray class. Initializes and builds the suffix array for the given input string.
SuffixArray::SuffixArray(const std::string& input_string) {
    // turn the input string into a vector of ints, which allows for a higher range of characters
    // necessary for the recusrive S1 calling, which creates more unique characters than char can handle
    constructS(input_string);

    // Construct the type array (T-type array) which classifies each character in the input string as either L-type or S-type.
    std::vector<bool> typeTArray = constructTTypeArray();
    // Use T-type array to Construct the sample pointer array. This array is used in the induced sorting of LMS substrings.
    std::vector<size_t> samplePointerArray = constructSamplePointerArray(typeTArray);

    // character counts can be directly used to create the bucket array
    std::map<size_t, size_t> charCounts = calcCharCounts();
    std::unordered_map<size_t, size_t> charToBucket;
    std::vector<size_t> buckets = initBuckets(charCounts, charToBucket);


    // Perform induced sorting on LMS (Left-Most S-type) substrings. .
    inducedSort(samplePointerArray, charCounts, buckets, charToBucket, typeTArray);

    // Check if all the characters in the reduced string S1 are unique.
    // If all characters in S1 are unique, directly compute the suffix array SA1.
    // Otherwise, recursively construct the suffix array for the reduced string S1.
    bool areAllLettersUnique;
    std::vector<size_t> S1 = constructS1AndCheckAllUniqueLetters(samplePointerArray, buckets, areAllLettersUnique);
    if (areAllLettersUnique){ 
        inducedSort(S1, samplePointerArray, charCounts, buckets, charToBucket, typeTArray);
    } else {
        SuffixArray suffixArray(S1);
        inducedSort(suffixArray.getSA(), samplePointerArray, charCounts, buckets, charToBucket, typeTArray);
    }

    // Construct the enchanced LCP (Longest Common Prefix) array
    // Used for optimilization of search

    constructLCPArray();
}

// overload for accepted vector of ints as an input through recursive calls
// LCP also is not needed to contruct so it is ommitted
SuffixArray::SuffixArray(const std::vector<size_t>& S0) {
    S = S0;
    std::vector<bool> typeTArray = constructTTypeArray();
    std::vector<size_t> samplePointerArray = constructSamplePointerArray(typeTArray);
    std::map<size_t, size_t> charCounts = calcCharCounts();
    std::unordered_map<size_t, size_t> charToBucket;
    std::vector<size_t> buckets = initBuckets(charCounts, charToBucket);
    inducedSort(samplePointerArray, charCounts, buckets, charToBucket, typeTArray);
    bool areAllLettersUnique = true;

    std::vector<size_t> S1 = constructS1AndCheckAllUniqueLetters(samplePointerArray, buckets, areAllLettersUnique);
    if (areAllLettersUnique){ 

        inducedSort(S1, samplePointerArray, charCounts, buckets, charToBucket, typeTArray);

    } else {
        SuffixArray suffixArray(S1);
        inducedSort(suffixArray.SA, samplePointerArray, charCounts, buckets, charToBucket, typeTArray);
    }
}

std::vector<size_t> SuffixArray::getSA(){return SA;}

std::vector<size_t> SuffixArray::search(const std::string &pattern) const{
    // Prepare the pattern to be searched by turning it into a vector of ints
    // the same way the string was processed
    if (pattern.empty())
        return {};

    std::vector<size_t> patternInt;
    patternInt.reserve(pattern.length());
    std::transform(pattern.begin(), pattern.end(), std::back_inserter(patternInt), [](char c) {
        return static_cast<size_t>(c - '\0');
    });

    // search logic is within the private part of the class
    return searchPrivate(patternInt);
}

// Convert string to vector of ints and append sentinel
void SuffixArray::constructS(const std::string &str) {  
    S.reserve(str.length() + 1);

    std::transform(str.begin(), str.end(), std::back_inserter(S), [](char c) {
        return static_cast<size_t>(c - '\0');
    });

    S.push_back(0);
}


// Two functions used to construct the buckets
std::map<size_t, size_t> SuffixArray::calcCharCounts() const{
    std::map<size_t, size_t> charCounts;
    for (size_t c : S) 
        charCounts[c]++;
    return charCounts;
}

std::vector<size_t> SuffixArray::initBuckets(
    const std::map<size_t, size_t> &charCounts, 
    std::unordered_map<size_t, size_t> &charToBucket
    ) const
{
    size_t curIndex = 0;
    std::vector<size_t> buckets;
    for (auto &pair : charCounts) {
        charToBucket[pair.first] = buckets.size();
        buckets.push_back(curIndex);
        curIndex += pair.second;
    }
    return buckets;
}

// true = S-type
// false = L-type
std::vector<bool> SuffixArray::constructTTypeArray() const{   
    if (S.size() == 1) return {true};

    std::vector<bool> typeTArray(S.size());

    // Initialize the last character as S-type
    typeTArray[S.size() - 1] = true;

    // Right-to-left iteration
    size_t i = S.size() - 2;
    while (true) {
        typeTArray[i] = (S[i] < S[i + 1]) || (S[i] == S[i + 1] && typeTArray[i + 1]);
        if (i == 0) break;
        i--;
    }

    return typeTArray;
}

std::vector<size_t> SuffixArray::constructSamplePointerArray(const std::vector<bool> &typeTArray) const{
    std::vector<size_t> samplePointerArray;

    // when first char is S type
    if (typeTArray[0] == true)
        samplePointerArray.push_back(0);

    for (size_t i = 1; i < S.size(); i++) {
        if (typeTArray[i] == true && !typeTArray[i - 1]) { // Current is S-type and previous is L-type
            samplePointerArray.push_back(i);
        }
    }
    return samplePointerArray;
}

std::vector<size_t> SuffixArray::initTails(const std::vector<size_t> &buckets) const{
    std::vector<size_t> tails(buckets.size());
    for (size_t i = 1; i < buckets.size(); i++) 
        tails[i - 1] = buckets[i] - 1;
    tails[buckets.size() - 1] = S.size() - 1;
    return tails;
}

void SuffixArray::induceSuffixes(
    const std::vector<size_t> &buckets,
    const std::vector<bool> &typeTArray,
    std::unordered_map<size_t, size_t> &charToBucket
) {
    std::vector<size_t> heads(buckets);
    std::vector<size_t> tails = initTails(buckets);

    for (size_t i = 0; i < S.size(); i++) {
        if (SA[i] == 0) continue;
        if (typeTArray[SA[i] - 1] == false) // if previous suffix is L type
            SA[heads[charToBucket[S[SA[i] - 1]]]++] = SA[i] - 1;
    }

    size_t i = S.size() - 1;
    while (true){
        if (SA[i] != 0)
            if (typeTArray[SA[i] - 1] == true) // if previous suffix is S type
                SA[tails[charToBucket[S[SA[i] - 1]]]--] = SA[i] - 1;
        if (i == 0) break;
        i--;
    }  
}

void SuffixArray::inducedSortCommon(
    std::map<size_t, size_t> &charCounts,
    const std::vector<size_t> &buckets, 
    std::unordered_map<size_t, size_t> &charToBucket
){
    if (!SA.empty())
        std::fill(SA.begin(), SA.end(), 0);
    else
        SA.resize(S.size(), 0);

    for (size_t i = 0; i < S.size(); i++)
        if (charCounts[S[i]] == 1)
            SA[buckets[charToBucket[S[i]]]] = i;
}

void SuffixArray::inducedSort(
    const std::vector<size_t> &SA1, 
    const std::vector<size_t> &samplePointerArray,   
    std::map<size_t, size_t> &charCounts,
    const std::vector<size_t> &buckets, 
    std::unordered_map<size_t, size_t> &charToBucket,
    const std::vector<bool> &typeTArray) 
{
    inducedSortCommon(charCounts, buckets, charToBucket);
    std::vector<size_t> tails = initTails(buckets);

    size_t i = SA1.size() - 1;
    while (true){
        size_t indexToLoad = samplePointerArray[SA1[i]];  // get the index that will be inserted into SuffixArray
        SA[tails[charToBucket[S[indexToLoad]]]--] = indexToLoad; // load into tail of bucket, while also moving the tail
        if (i == 0) break;
        i--;
    }
    induceSuffixes(buckets, typeTArray, charToBucket);
}

void SuffixArray::inducedSort(
    const std::vector<size_t> &samplePointerArray, 
    std::map<size_t, size_t> &charCounts,
    const std::vector<size_t> &buckets, 
    std::unordered_map<size_t, size_t> &charToBucket,
    const std::vector<bool> &typeTArray) 
{

    inducedSortCommon(charCounts, buckets, charToBucket);

    std::vector<size_t> tails = initTails(buckets);

    for (size_t i = 0; i < samplePointerArray.size(); i++) {
        size_t indexToLoad = samplePointerArray[i]; 
        SA[tails[charToBucket[S[indexToLoad]]]--] = indexToLoad;
    }

    induceSuffixes(buckets, typeTArray, charToBucket);
}

size_t SuffixArray::getLMSSubstrLen(size_t SPAIndex, const std::vector<size_t> &samplePointerArray) const{
    if (SPAIndex == samplePointerArray.size() - 1)
        return S.size() - samplePointerArray[SPAIndex];
    return samplePointerArray[SPAIndex + 1] - samplePointerArray[SPAIndex] + 1;
}

std::vector<size_t> SuffixArray::constructS1AndCheckAllUniqueLetters(
    const std::vector<size_t> &samplePointerArray,
    const std::vector<size_t> &buckets,    
    bool &areAllLettersUnique
) const
{
    std::vector<size_t> S1(samplePointerArray.size());

    // Pre-building the map
    std::unordered_map<size_t, size_t> SPAReverseMap;
    for (size_t i = 0; i < samplePointerArray.size(); i++)
        SPAReverseMap[samplePointerArray[i]] = i;

    size_t curBucketIndex = 0, prevFoundLMSCharIndex = 0, prevSubstrLen = 0, bucketStart, bucketEnd;
    bool firstFound = false;

    for (size_t bucketIdx = 0; bucketIdx < buckets.size(); bucketIdx++) {
        bucketStart = buckets[bucketIdx];
        bucketEnd = (bucketIdx + 1 == buckets.size()) ? S.size() - 1 : buckets[bucketIdx + 1] - 1;

        bool foundInBucket = false;
        for (size_t j = bucketStart; j <= bucketEnd; j++) {
            auto it = SPAReverseMap.find(SA[j]);
            if (it == SPAReverseMap.end()) continue;

            size_t curSubstrLen = getLMSSubstrLen(it->second, samplePointerArray);

            if (firstFound)
                if (foundInBucket) {
                    if (curSubstrLen == prevSubstrLen && std::equal(S.begin() + SA[prevFoundLMSCharIndex], S.begin() + SA[prevFoundLMSCharIndex] + curSubstrLen, S.begin() + SA[j])) {
                        areAllLettersUnique = false;
                        S1[it->second] = curBucketIndex - 1;
                        continue;
                    }
                }

            prevFoundLMSCharIndex = j;
            prevSubstrLen = curSubstrLen;
            S1[it->second] = ++curBucketIndex - 1;
            foundInBucket = true;
            firstFound = true;
        }
    }

    return S1;
}



size_t SuffixArray::LCPRec(const size_t i, const size_t j){
    size_t res;
    size_t mid = (i + j) / 2;
    if (j - i == 1)
        return LCP[j];
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
    LCPRec(0, S.size() - 1);
}

size_t SuffixArray::getLCP(const size_t index1, const size_t index2) const{
    if (index1 + 1 == index2)
        return LCP[index2];
    else
        return LCP[S.size() + ((index1 + index2) / 2)];
}

size_t SuffixArray::countMatches(const std::vector<size_t> &pattern, size_t startIndex, size_t patternLength, size_t stringIndex) const{
    size_t matches = startIndex;
    for (;matches < patternLength; matches++) {
        if (S[stringIndex + matches] != pattern[matches])
            break;
    }
    return matches;
}

std::vector<size_t> SuffixArray::findAllOccurances(const size_t suffixArrayMatchIndex, const size_t patternLength) const{
    std::vector<size_t> matches = {SA[suffixArrayMatchIndex]};

    // check to the left of intial match
    size_t suffixArrayIndex = suffixArrayMatchIndex;
    while (true) {
        if (suffixArrayIndex == 0)
            break;
        if (LCP[suffixArrayIndex] >= patternLength)
            matches.push_back(SA[suffixArrayIndex - 1]);
        else
            break;
        suffixArrayIndex--;
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

std::vector<size_t> SuffixArray::searchPrivate(const std::vector<size_t> &pattern) const{
    size_t low = 0, high = SA.size() - 1;
    size_t lowMatches = countMatches(pattern, 0, pattern.size(), SA[low]);
    size_t highMatches = countMatches(pattern, 0, pattern.size(), SA[high]);

    if (highMatches == pattern.size())
        return findAllOccurances(high, pattern.size());

    if (lowMatches == pattern.size())
        return findAllOccurances(low, pattern.size());

    size_t mid;
    size_t lcpLow, lcpHigh;
    size_t maxMatches;

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
            maxMatches = countMatches(pattern, maxMatches, pattern.size(), SA[mid]);

            if (maxMatches == pattern.size()) // if exact match occurs
                return findAllOccurances(mid, pattern.size());
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