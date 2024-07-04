// PasswordComplexityEstimator.cpp

#include "PasswordComplexityEstimator.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

PasswordComplexityEstimator::PasswordComplexityEstimator(const std::string& DatasetPath) {
    std::ifstream file(DatasetPath);
    std::string line;
    while (std::getline(file, line)) {
        Passwords.push_back(line);
    }

    if (Passwords.empty()) {
        throw std::runtime_error("Dataset is empty or file not found");
    }
}

PasswordComplexityEstimate PasswordComplexityEstimator::Estimate(const std::string& Password) {
    double entropy = CalculateEntropy(Password);

    uint_seconds ttd = uint_seconds::max();
    try {
        ttd = CalculateTTD(entropy);
    }
    catch (const std::overflow_error&) {
        // TTD max if overflow
    }

    PasswordComplexityTiers tier = DetermineTier(ttd);

    return { entropy, ttd, tier };
}

size_t PasswordComplexityEstimator::CountPrefixed(const std::string& Prefix) {
    auto start = std::lower_bound(Passwords.begin(), Passwords.end(), Prefix);
    auto end = std::upper_bound(Passwords.begin(), Passwords.end(), Prefix + std::string(100, 'z'));
    return std::distance(start, end);
}

double PasswordComplexityEstimator::CalculateProbability(const std::string& GivenPrefix, char ExpectingNextChar) {
    size_t total = CountPrefixed(GivenPrefix);
    size_t matched = CountPrefixed(GivenPrefix + ExpectingNextChar);
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(matched) / total;
}

double PasswordComplexityEstimator::CalculateEntropy(const std::string& Password) {
    if (std::any_of(Password.begin(), Password.end(), [](char c) { return PASSWORD_ALLOWED_CHARS.find(c) == std::string::npos; })) {
        throw std::invalid_argument("Password contains invalid characters");
    }

    std::string prefix;
    double entropy = 0.0;

    for (char token : Password) {
        double chosen_token_prob;
        while (true) {
            chosen_token_prob = CalculateProbability(prefix, token);

            if (chosen_token_prob == 0.0) {
                if (prefix.empty()) {
                    chosen_token_prob = 1e-9; // Ultra Extreme tier for sure
                    break;
                }
                prefix = prefix.substr(1);
            }
            else {
                break;
            }
        }

        entropy += -std::log2(chosen_token_prob);
        prefix += token;
    }

    return entropy;
}

uint_seconds PasswordComplexityEstimator::CalculateTTD(double Entropy) const {
    uint64_t result = 1;
    const uint64_t base = 2;
    double integralPart;
    double fractionalPart = modf(Entropy, &integralPart);

    for (int i = 0; i < integralPart; i++) {
        if (result > (std::numeric_limits<uint64_t>::max() / base)) {
            throw std::overflow_error("Overflow in calculating TTD");
        }
        result *= base;
    }

    result = static_cast<uint64_t>(result * std::exp2(fractionalPart));

    return uint_seconds(result / 100000000);
}

PasswordComplexityTiers PasswordComplexityEstimator::DetermineTier(const uint_seconds& Ttd) {
    const uint64_t seconds_per_day = 86400;

    const std::vector<PasswordComplexityTiers> complexities = {
        PasswordComplexityTiers::PATHETIC,
        PasswordComplexityTiers::LOW,
        PasswordComplexityTiers::MEDIUM,
        PasswordComplexityTiers::HIGH,
        PasswordComplexityTiers::EXTREME
    };

    for (int i = 0; i < complexities.size() - 1; ++i) {
        uint64_t threshold_days = 365 * std::pow(10, i - 2);
        uint_seconds threshold = uint_seconds(threshold_days * seconds_per_day);

        if (Ttd < threshold) {
            return complexities[i];
        }
    }

    return complexities.back();
}
