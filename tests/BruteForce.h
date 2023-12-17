#include <vector>
#include <string>
#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

class BruteForce {
public:
    explicit BruteForce(const std::string& string);
	
	std::vector<size_t> search(const std::string & pattern) const;

private:
    std::string string;
};
#endif