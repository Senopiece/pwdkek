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
    ULTRA_EXTREME
};

struct PasswordComplexityEstimate {
    double entropy;
    std::chrono::duration<double> ttd;
    PasswordComplexityTiers tier;
};

class PasswordComplexityEstimator {
public:
    PasswordComplexityEstimator(const std::string& dataset_path = "datasets/rockyou-utf8-sorted.txt.gz") {
        igzstream file(dataset_path.c_str());
        std::string line;
        while (std::getline(file, line)) {
            passwords.push_back(line);
        }
    }

    PasswordComplexityEstimate estimate(const std::string& password) {
        double entropy = calculateEntropy(password);

        std::chrono::duration<double> ttd = std::chrono::duration<double>::max();
        try {
            ttd = calculateTTD(entropy);
        } catch (const std::overflow_error&) {
            // Handle overflow
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

    std::chrono::duration<double> calculateTTD(double entropy) {
        return std::chrono::duration<double>(10e-9 * std::pow(2, entropy));
    }

    PasswordComplexityTiers determineTier(const std::chrono::duration<double>& ttd) {
        static const std::vector<PasswordComplexityTiers> COMPLEXITIES = {
            PasswordComplexityTiers::PATHETIC, 
            PasswordComplexityTiers::LOW, 
            PasswordComplexityTiers::MEDIUM, 
            PasswordComplexityTiers::HIGH, 
            PasswordComplexityTiers::EXTREME, 
            PasswordComplexityTiers::ULTRA_EXTREME
        };

        if (ttd == std::chrono::duration<double>::max()) {
            return COMPLEXITIES.back();
        }

        for (size_t i = 0; i < COMPLEXITIES.size() - 2; ++i) {
            auto threshold = std::chrono::duration<double>(365 * std::pow(10, i - 2) * 24 * 60 * 60);
            if (ttd < threshold) {
                return COMPLEXITIES[i];
            }
        }

        return COMPLEXITIES[COMPLEXITIES.size() - 2];
    }
};

int main() {
    std::cout << "Loading..." << std::endl;
    PasswordComplexityEstimator estimator;

    try {
        while (true) {
            std::cout << "\nEnter a password: ";
            std::string input;
            std::getline(std::cin, input);
            if (input.empty()) break;

            try {
                PasswordComplexityEstimate estimate = estimator.estimate(input);

                std::cout << "Password entropy: " << estimate.entropy << std::endl;

                auto ttd = estimate.ttd;

                // Define days as 24 hours
                using days = std::chrono::duration<int, std::ratio<86400>>;
                
                auto days_count = std::chrono::duration_cast<days>(ttd).count();
                int years = days_count / 365;
                days_count %= 365;
                auto hours = std::chrono::duration_cast<std::chrono::hours>(ttd).count() % 24;
                auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ttd).count() % 60;
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ttd).count() % 60;

                std::cout << "Time to decode with 1Gh/s: "
                          << years << " years " << days_count << " days " 
                          << hours << " hours " << minutes << " minutes " << seconds << " seconds" 
                          << std::endl;
                std::cout << "Tier: " << static_cast<int>(estimate.tier) << std::endl;

            } catch (const std::invalid_argument& e) {
                std::cerr << e.what() << std::endl;
                continue;
            }
        }
    } catch (const std::exception&) {
        // Handle any other exceptions
    }

    return 0;
}
