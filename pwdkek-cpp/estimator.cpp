#include "estimator.h"

std::ostream& operator<<(std::ostream& os, const PasswordComplexityTiers& tier) {
    switch (tier) {
        case PasswordComplexityTiers::PATHETIC: return os << "Pathetic";
        case PasswordComplexityTiers::LOW: return os << "Low";
        case PasswordComplexityTiers::MEDIUM: return os << "Medium";
        case PasswordComplexityTiers::HIGH: return os << "High";
        case PasswordComplexityTiers::EXTREME: return os << "Extreme";
        default: return os << "Unknown Tier";
    }
}

PasswordComplexityEstimator::PasswordComplexityEstimator(const std::string& dataset_path) {
    igzstream file(dataset_path.c_str());
    std::string line;
    while (std::getline(file, line)) {
        passwords.push_back(line);
    }

    if (passwords.empty()) {
        throw std::runtime_error("something is wrong with the dataset, maybe file not found");
    }
}

PasswordComplexityEstimate PasswordComplexityEstimator::estimate(const std::string& password) {
    double entropy = calculateEntropy(password);

    uint_seconds ttd = uint_seconds::max();
    try {
        ttd = calculateTTD(entropy);
    } catch (const std::overflow_error&) {
        // TTD max if overflow
    }

    PasswordComplexityTiers tier = determineTier(ttd);

    return {entropy, ttd, tier};
}

size_t PasswordComplexityEstimator::countPrefixed(const std::string& prefix) {
    auto start = std::lower_bound(passwords.begin(), passwords.end(), prefix);
    auto end = std::upper_bound(passwords.begin(), passwords.end(), prefix + std::string(100, 'z'));
    return std::distance(start, end);
}

double PasswordComplexityEstimator::calculateProbability(const std::string& given_prefix, char expecting_next_char) {
    size_t total = countPrefixed(given_prefix);
    size_t matched = countPrefixed(given_prefix + expecting_next_char);
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(matched) / total;
}

double PasswordComplexityEstimator::calculateEntropy(const std::string& password) {
    if (std::any_of(password.begin(), password.end(), [](char c) { return PASSWORD_ALLOWED_CHARS.find(c) == std::string::npos; })) {
        throw std::invalid_argument("Password contains invalid characters");
    }

    std::string prefix;
    double entropy = 0.0;

    for (char token : password) {
        double chosen_token_prob;
        while (true) {
            chosen_token_prob = calculateProbability(prefix, token);

            if (chosen_token_prob == 0.0) {
                if (prefix.empty()) {
                    chosen_token_prob = 1e-9; // Ultra Extreme tier for sure
                    break;
                }
                prefix = prefix.substr(1);
            } else {
                break;
            }
        }

        entropy += -std::log2(chosen_token_prob);
        prefix += token;
    }

    return entropy;
}

uint_seconds PasswordComplexityEstimator::calculateTTD(double entropy) const {
    uint64_t result = 1;
    const uint64_t base = 2;
    double integralPart;
    double fractionalPart = modf(entropy, &integralPart);

    // Calculate 2^integralPart safely
    for (int i = 0; i < integralPart; i++) {
        if (result > (std::numeric_limits<uint64_t>::max() / base)) {
            throw std::overflow_error("Overflow in calculating TTD");
        }
        result *= base;
    }

    // Adjust for fractional part using exp2 (safe since result < max_uint64)
    result = static_cast<uint64_t>(result * std::exp2(fractionalPart));

    // Now divide by 10^9 to convert to seconds (assuming result is in nanoseconds)
    return uint_seconds(result / 100000000);
}

PasswordComplexityTiers PasswordComplexityEstimator::determineTier(const uint_seconds& ttd) {
    const uint64_t seconds_per_day = 86400;

    const std::vector<PasswordComplexityTiers> COMPLEXITIES = {
        PasswordComplexityTiers::PATHETIC,
        PasswordComplexityTiers::LOW,
        PasswordComplexityTiers::MEDIUM,
        PasswordComplexityTiers::HIGH,
        PasswordComplexityTiers::EXTREME
    };

    for (int i = 0; i < COMPLEXITIES.size() - 1; ++i) {
        uint64_t threshold_days = 365 * std::pow(10, i - 2);
        uint_seconds threshold = uint_seconds(threshold_days * seconds_per_day);

        if (ttd < threshold) {
            return COMPLEXITIES[i];
        }
    }

    return COMPLEXITIES.back();
}
