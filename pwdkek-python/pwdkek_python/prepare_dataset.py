import gzip
import sys
from pathlib import Path

from pwdkek_python.complexity_estimator import PASSWORD_ALLOWED_CHARS


def prepare_dataset(input_path: str | Path, output_path: str | Path) -> None:
    with open(input_path, "r") as file:
        passwords = [
            "".join([ch for ch in line if ch in PASSWORD_ALLOWED_CHARS])
            for line in file.readlines()
        ]

    passwords = list(filter(lambda pwd: len(pwd) != 0, passwords))
    passwords.sort()

    with gzip.open(output_path, "wt") as file:
        for password in passwords:
            file.write(password + "\n")


if __name__ == "__main__":
    # NOTE: convert input to utf-8 beforehand
    prepare_dataset(sys.argv[1], sys.argv[2])
