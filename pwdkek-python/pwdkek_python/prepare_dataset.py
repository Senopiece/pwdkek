import gzip
import sys
from pwdkek_python.complexity_estimator import PASSWORD_ALLOWED_CHARS


if __name__ == "__main__":
    # NOTE: convert input to utf-8 beforehand
    with open(sys.argv[1], "r") as file:
        passwords = [
            "".join([ch for ch in line if ch in PASSWORD_ALLOWED_CHARS])
            for line in file.readlines()
        ]

    passwords = list(filter(lambda pwd: len(pwd) != 0, passwords))
    passwords.sort()

    with gzip.open(sys.argv[2], "wt") as file:
        for password in passwords:
            file.write(password + "\n")
