#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include <chrono>
#include <stdexcept>
#include "gzstream.h"

const std::string PASSWORD_ALLOWED_CHARS = 
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

enum class PasswordComplexityTiers {
    PATHETIC,
    LOW,
    MEDIUM,
    HIGH,
    EXTREME,
};

using uint_seconds = std::chrono::duration<uint64_t>;

std::ostream& operator<<(std::ostream& os, const PasswordComplexityTiers& tier);

struct PasswordComplexityEstimate {
    double entropy;
    uint_seconds ttd;
    PasswordComplexityTiers tier;
};

class PasswordComplexityEstimator {
public:
    PasswordComplexityEstimator(const std::string& dataset_path);

    PasswordComplexityEstimate estimate(const std::string& password);

private:
    std::vector<std::string> passwords;

    size_t countPrefixed(const std::string& prefix);

    double calculateProbability(const std::string& given_prefix, char expecting_next_char);

    double calculateEntropy(const std::string& password);

    uint_seconds calculateTTD(double entropy) const;

    static PasswordComplexityTiers determineTier(const uint_seconds& ttd);
};

#endif // ESTIMATOR_H
