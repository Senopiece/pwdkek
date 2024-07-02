import bisect
from datetime import timedelta
from math import log2

print("Loading...")
with open("datasets/rockyou-utf8-sorted.txt", "r") as file:
    passwords = file.readlines()


def count_prefixed(prefix: str):
    start = bisect.bisect_left(passwords, prefix)
    end = bisect.bisect_right(passwords, prefix + "z" * 100)
    return end - start


def probability(given_prefix: str, expecting_next_char: str):
    total = count_prefixed(given_prefix)
    matched = count_prefixed(given_prefix + expecting_next_char)
    return matched / total


def calc_entropy(password: str):
    password += "\n"
    prefix = ""
    entropy = 0

    for token in password:
        while True:
            chosen_token_prob = probability(prefix, token)

            if chosen_token_prob == 0:
                if prefix == "":
                    chosen_token_prob = 0.000000001  # A Legendary tier for sure
                    break
                prefix = ""
            else:
                break

        entropy += -log2(chosen_token_prob)
        prefix += token

    return entropy


entropy = calc_entropy(input("Enter a password: "))
print("Password entropy:", entropy)

tiers = ["Pathetic", "Low", "Medium", "High", "Extreme"]


def tier(ttd: timedelta):
    for i, tier in enumerate(tiers[:-1]):
        threshold = timedelta(days=365 * pow(10, i))

        if ttd < threshold:
            return tier

    return tiers[-1]


try:
    ttd = timedelta(seconds=10e-9 * pow(2, entropy))
    years = ttd.days // 365
    days = ttd.days % 365
    hours, remainder = divmod(ttd.seconds, 3600)
    minutes, seconds = divmod(remainder, 60)
    print(
        "Time to decode with 1Gh/s:",
        f"{years} years {days} days {hours} hours {minutes} minutes {seconds} seconds",
    )
    print("Tier:", tier(ttd))
except OverflowError:
    print("Legendary Tier")
