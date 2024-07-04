import argparse
from datetime import timedelta

from pwdkek_python.complexity_estimator import PasswordComplexityEstimator


def main():
    parser = argparse.ArgumentParser(description="Password Complexity Estimator")
    parser.add_argument(
        "--dataset_path",
        type=str,
        help="Path to the dataset file",
        default="datasets/rockyou-utf8-filtered-sorted.txt.gz",
    )
    args = parser.parse_args()

    print("Loading...")
    try:
        estimator = PasswordComplexityEstimator(args.dataset_path)
    except ValueError as e:
        print("Error:", e)
        return

    try:
        while True:
            print()
            try:
                estimate = estimator.estimate(input("Enter a password: "))
            except ValueError as e:
                print(e)
                continue

            print("Password entropy:", estimate.entropy)

            print("Time to decode with 1Gh/s: ", end="")
            if estimate.ttd == timedelta.max:
                print("Uncountable number of years")
            else:
                ttd = estimate.ttd
                years = ttd.days // 365
                days = ttd.days % 365
                hours, remainder = divmod(ttd.seconds, 3600)
                minutes, seconds = divmod(remainder, 60)
                print(
                    f"{years} years {days} days {hours} hours {minutes} minutes {seconds} seconds",
                )

            print("Tier:", estimate.tier.value)

    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
