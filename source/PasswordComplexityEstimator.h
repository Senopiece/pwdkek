// PasswordComplexityEstimator.h

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <string>
#include <chrono>

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

struct PasswordComplexityEstimate {
    double Entropy;
    uint_seconds Ttd;
    PasswordComplexityTiers Tier;
};

class PasswordComplexityEstimator {
public:
    PasswordComplexityEstimator(const std::string& DatasetPath);

    PasswordComplexityEstimate Estimate(const std::string& Password);

private:
    std::vector<std::string> Passwords;

    size_t CountPrefixed(const std::string& Prefix);
    double CalculateProbability(const std::string& GivenPrefix, char ExpectingNextChar);
    double CalculateEntropy(const std::string& Password);
    uint_seconds CalculateTTD(double Entropy) const;
    static PasswordComplexityTiers DetermineTier(const uint_seconds& Ttd);
};
