#ifndef SUFFIXARRAY_H
#define SUFFIXARRAY_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

class SuffixArray {
public:
    SuffixArray(const std::string& input_string);
    SuffixArray(const std::vector<int>& S0);
    std::vector<int> search(const std::string &pattern);
    std::string getTypeArrayString();
    std::vector<int> getSamplePointerArray();
    std::vector<int> getSA();
    void printTable();

private:
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

    void constructS(const std::string &str);
    void calcCharCounts();
    void initBuckets();
    void constructTTypeArray();
    void constructSamplePointerArray();
    int getLMSSubstrLen(size_t SPAIndex);
    bool constructS1AndCheckAllUniqueLetters();
    std::vector<int> findAllOccurances(int suffixArrayMatchIndex, int patternLength);
    std::vector<int> getSA1();
    void inducedSort(std::vector<int> arrToLoad, bool LMSInduction);
    int LCPRec(int i, int j);
    void constructLCPArray();
    int getLCP(int index1, int index2);
    int countMatches(std::vector<int> pattern, int startIndex, int patternLength, int stringIndex);
    std::vector<int> searchPrivate (std::vector<int> pattern);
};

#endif // SUFFIXARRAY_H