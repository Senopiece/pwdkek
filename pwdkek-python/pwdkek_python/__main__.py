import argparse
from datetime import timedelta

from pwdkek_python.complexity_estimator import PasswordComplexityEstimator
from pwdkek_python.builtin_datasets import BuiltInDataset


def main():
    parser = argparse.ArgumentParser(description="Password Complexity Estimator")
    parser.add_argument(
        "--dataset",
        type=str,
        help="Built-in dataset name or path to the dataset file",
        default="small",
    )
    args = parser.parse_args()

    print("Loading...")
    try:
        if args.dataset in BuiltInDataset.names():
            dataset = BuiltInDataset[args.dataset.upper()]
        else:
            dataset = args.dataset
        estimator = PasswordComplexityEstimator(dataset)
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
