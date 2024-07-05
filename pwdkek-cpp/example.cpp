#include <iostream>
#include <string>
#include "estimator.h"

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
