// MyBlueprintFunctionLibrary.cpp

#include "MyBlueprintFunctionLibrary.h"
#include "PasswordComplexityEstimator.h"

float UMyBlueprintFunctionLibrary::CalculatePasswordComplexity(const FString& Password) {
    PasswordComplexityEstimator Estimator("datasets/data.txt");

    PasswordComplexityEstimate Estimate = Estimator.Estimate(TCHAR_TO_UTF8(*Password));
    return static_cast<float>(Estimate.Entropy);
}
