#ifndef SUFFIXARRAY_H
#define SUFFIXARRAY_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

class SuffixArray {
public:
    explicit SuffixArray(const std::string& input_string);
    explicit SuffixArray(const std::vector<size_t>& S0);
    std::vector<size_t> search(const std::string &pattern) const;
    std::vector<size_t> getSA();

private:
    std::vector<size_t> S;
    std::vector<size_t> SA;
    std::vector<size_t> LCP;

    void constructS(const std::string &str);
    std::map<size_t, size_t> calcCharCounts() const;
    std::vector<size_t> initBuckets(const std::map<size_t, size_t> &charCounts, std::unordered_map<size_t, size_t> &charToBucket) const;
    std::vector<bool> constructTTypeArray() const;
    std::vector<size_t> constructSamplePointerArray(const std::vector<bool> &typeTArray) const;
    std::vector<size_t> initTails(const std::vector<size_t> &buckets) const;
    void induceSuffixes(
        const std::vector<size_t> &buckets,
        const std::vector<bool> &typeTArray,
        std::unordered_map<size_t, size_t> &charToBucket
    );

    void inducedSortCommon(
        std::map<size_t, size_t> &charCounts,
        const std::vector<size_t> &buckets, 
        std::unordered_map<size_t, size_t> &charToBucket
    );

    void inducedSort(
        const std::vector<size_t> &SA1, 
        const std::vector<size_t> &samplePointerArray,   
        std::map<size_t, size_t> &charCounts,
        const std::vector<size_t> &buckets, 
        std::unordered_map<size_t, size_t> &charToBucket,
        const std::vector<bool> &typeTArray
    );

    void inducedSort(
        const std::vector<size_t> &samplePointerArray, 
        std::map<size_t, size_t> &charCounts,
        const std::vector<size_t> &buckets, 
        std::unordered_map<size_t, size_t> &charToBucket,
        const std::vector<bool> &typeTArray
    ); 

    size_t getLMSSubstrLen(const size_t SPAIndex, const std::vector<size_t> &samplePointerArray) const;

    std::vector<size_t> constructS1AndCheckAllUniqueLetters(
        const std::vector<size_t> &samplePointerArray,
        const std::vector<size_t> &buckets,    
        bool &areAllLettersUnique
    ) const;

    std::vector<size_t> findAllOccurances(const size_t suffixArrayMatchIndex, const size_t patternLength) const;
    size_t LCPRec(const size_t i, const size_t j);
    void constructLCPArray();
    size_t getLCP(const size_t index1, const size_t index2) const;
    size_t countMatches(
        const std::vector<size_t> &pattern, 
        const size_t startIndex, 
        const size_t patternLength, 
        const size_t stringIndex) const;
    std::vector<size_t> searchPrivate(const std::vector<size_t> &pattern) const;
};

#endif // SUFFIXARRAY_H