from datetime import timedelta
from estimator import *
import json
import argparse

# Функция для загрузки паролей и их тиров из JSON файла
def load_passwords(filename):
    with open(filename, 'r', encoding='utf-8') as file:
        data = json.load(file)
    return data

# Функция для тестирования паролей
def test_passwords(filename):
    parser = argparse.ArgumentParser(description="Password Complexity Estimator Testing")
    parser.add_argument(
        "--dataset_path",
        type=str,
        help="Path to the dataset file",
        default="datasets/rockyou-utf8-filtered-sorted.txt.gz",
    )
    args = parser.parse_args()

    password_data = load_passwords(filename)
    estimator = PasswordComplexityEstimator(args.dataset_path)

    # Количество совпадений по тирам
    correct_predictions = {tier_name.value : 0 for tier_name in PasswordComplexityTiers}
    
    for expected_tier, passwords in password_data.items():
        print(f"Testing {expected_tier} passwords:")
        for password in passwords:
        
            try:
                estimate = estimator.estimate(password)
            except ValueError as e:
                print(e)
                continue
        
            entropy = estimate.entropy
            predicted_tier = estimate.tier.value
            ttd = estimate.ttd
            if ttd == timedelta.max:
                ttd = "Uncountable number of years"

            print(f"Password: {password}, Predicted Tier: {predicted_tier}, Entropy: {round(entropy, 3)}, TTD: {ttd}")

            if expected_tier == predicted_tier:
                correct_predictions[predicted_tier] += 1
        print('-' * 100)
    print("\nResults:")
    for tier_name, correct_count in correct_predictions.items():
        if tier_name in password_data:
            print(f"Correct predictions for {tier_name} tier: {correct_count / len(password_data[tier_name])}")
            
if __name__ == "__main__":
    test_passwords('test_passwords.json')
    