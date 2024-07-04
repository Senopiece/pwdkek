from datetime import timedelta
from password_entropy import tier, calculate_password_entropy, TIERS
import json

# Функция для загрузки паролей и их тиров из JSON файла
def load_passwords(filename):
    with open(filename, 'r', encoding='utf-8') as file:
        data = json.load(file)
    return data

# Функция для тестирования паролей
def test_passwords(filename):
    password_data = load_passwords(filename)

    # Количество совпадений по тирам
    correct_predictions = {tier_name : 0 for tier_name in TIERS}
    
    # cумма предсказаний для самых плохих паролей
    sum_pathetic_pred = 0
    tier_to_num = {tier_name.value : i for i, tier_name in enumerate(PasswordComplexityTiers)}
    pathetic_tier = [tier_name.value for tier_name in PasswordComplexityTiers][0]
    
    for expected_tier, passwords in password_data.items():
        print(f"Testing {expected_tier} passwords:")
        for password in passwords:
            entropy = calculate_password_entropy(password)
            ttd = timedelta(seconds=10e-9 * pow(2, entropy))
            predicted_tier = tier(ttd)

            print(f"Password: {password}, Predicted Tier: {predicted_tier}, Entropy: {entropy}")

            if expected_tier == predicted_tier:
                correct_predictions[predicted_tier] += 1   
            if expected_tier == pathetic_tier:
                sum_pathetic_pred += tier_to_num[predicted_tier]

    print("\nResults:")
    for tier_name, correct_count in correct_predictions.items():
        if tier_name in password_data:
            print(f"Correct predictions for {tier_name} tier: {correct_count / len(password_data[tier_name])}")
    
    print(f"\nAverage predictions for {pathetic_tier} passwords: {sum_pathetic_pred / len(password_data[pathetic_tier])}. must be {tier_to_num[pathetic_tier]}")

if __name__ == "__main__":
    test_passwords('test_passwords.json')