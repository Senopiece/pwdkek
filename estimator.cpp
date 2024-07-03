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

struct PasswordComplexityEstimate {
    double entropy;
    uint_seconds ttd;
    PasswordComplexityTiers tier;
};

class PasswordComplexityEstimator {
public:
    PasswordComplexityEstimator(const std::string& dataset_path) {        
        igzstream file(dataset_path.c_str());
        std::string line;
        while (std::getline(file, line)) {
            passwords.push_back(line);
        }

        if (passwords.size() == 0) {
            throw std::runtime_error("something is wrong with the dataset, maybe file not found");
        }
    }

    PasswordComplexityEstimate estimate(const std::string& password) {
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

private:
    std::vector<std::string> passwords;

    size_t countPrefixed(const std::string& prefix) {
        auto start = std::lower_bound(passwords.begin(), passwords.end(), prefix);
        auto end = std::upper_bound(passwords.begin(), passwords.end(), prefix + std::string(100, 'z'));
        return std::distance(start, end);
    }

    double calculateProbability(const std::string& given_prefix, char expecting_next_char) {
        size_t total = countPrefixed(given_prefix);
        size_t matched = countPrefixed(given_prefix + expecting_next_char);
        if (total == 0) {
            return 0.0;
        }
        return static_cast<double>(matched) / total;
    }

    double calculateEntropy(const std::string& password) {
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

    uint_seconds calculateTTD(double entropy) const {
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

    static PasswordComplexityTiers determineTier(const uint_seconds& ttd) {
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
};

int main(int argc, char* argv[]) {
    std::string datasetPath = "datasets/rockyou-utf8-filtered-sorted.txt.gz"; // Default path

    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name
        std::string arg = argv[i];
        if (arg == "--dataset_path" && i + 1 < argc) {
            datasetPath = argv[++i]; // Increment i to skip the value
        }
    }

    try {
        std::cout << "Loading..." << std::endl;
        PasswordComplexityEstimator estimator(datasetPath);

        while (true) {
            std::cout << "\nEnter a password: ";
            std::string input;
            std::getline(std::cin, input);

            try {
                PasswordComplexityEstimate estimate = estimator.estimate(input);

                std::cout << "Password entropy: " << estimate.entropy << std::endl;

                auto ttd = estimate.ttd;
                std::cout << "Time to decode with 1Gh/s: ";
                if (ttd == uint_seconds::max()) {
                    std::cout << "Uncountable number of years";
                } else {
                    using days = std::chrono::duration<uint64_t, std::ratio<86400>>;
                    auto days_count = std::chrono::duration_cast<days>(ttd).count();
                    int years = days_count / 365;
                    days_count %= 365;
                    auto hours = std::chrono::duration_cast<std::chrono::hours>(ttd).count() % 24;
                    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ttd).count() % 60;
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ttd).count() % 60;

                    std::cout << years << " years " << days_count << " days " 
                              << hours << " hours " << minutes << " minutes " << seconds << " seconds";
                }
                std::cout << std::endl;

                std::cout << "Tier: " << estimate.tier << std::endl;

            } catch (const std::invalid_argument& e) {
                std::cerr << e.what() << std::endl;
                continue;
            }
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
