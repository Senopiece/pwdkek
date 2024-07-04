from datetime import timedelta

from pwdkek_python.builtin_datasets import BuiltInDataset
from pwdkek_python.complexity_estimator import (
    PasswordComplexityTiers,
    PasswordComplexityEstimator,
)
import json
import argparse


# Функция для загрузки паролей и их тиров из JSON файла
def load_passwords(filename):
    with open(filename, "r", encoding="utf-8") as file:
        data = json.load(file)
    return data


# Функция для тестирования паролей
def test_passwords(filename):
    parser = argparse.ArgumentParser(
        description="Password Complexity Estimator Testing"
    )
    parser.add_argument(
        "--dataset",
        type=str,
        help="Built-in dataset name or path to the dataset file",
        default="small",
    )
    args = parser.parse_args()

    password_data = load_passwords(filename)

    if args.dataset in BuiltInDataset.names():
        dataset = BuiltInDataset[args.dataset.upper()]
    else:
        dataset = args.dataset

    estimator = PasswordComplexityEstimator(dataset)

    # Количество совпадений по тирам
    correct_predictions = {tier_name.value: 0 for tier_name in PasswordComplexityTiers}

    # cумма предсказаний для самых плохих паролей
    sum_pathetic_pred = 0
    tier_to_num = {
        tier_name.value: i for i, tier_name in enumerate(PasswordComplexityTiers)
    }
    pathetic_tier = [tier_name.value for tier_name in PasswordComplexityTiers][0]

    # число правильно определeнных самых плохих паролей
    cnt_correct_pathetic_pred = 0

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

            print(
                f"Password: {password}, Predicted Tier: {predicted_tier}, Entropy: {round(entropy, 3)}, TTD: {ttd}"
            )

            if expected_tier == predicted_tier:
                correct_predictions[predicted_tier] += 1
            if expected_tier == pathetic_tier:
                sum_pathetic_pred += tier_to_num[predicted_tier]
                if expected_tier == predicted_tier:
                    cnt_correct_pathetic_pred += 1

        print("-" * 100)
    print("\nResults:")
    for tier_name, correct_count in correct_predictions.items():
        if tier_name in password_data:
            print(
                f"Correct predictions for {tier_name} tier: {correct_count / len(password_data[tier_name])}"
            )

    print(
        f"\nAverage predictions for {pathetic_tier} passwords: {sum_pathetic_pred / len(password_data[pathetic_tier])}. must be {tier_to_num[pathetic_tier]}"
    )
    print("Recall:", cnt_correct_pathetic_pred / len(password_data[pathetic_tier]))


if __name__ == "__main__":
    test_passwords("tests/test_files/test_passwords.json")
