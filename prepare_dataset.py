import gzip
import string
import sys


PASSWORD_ALLOWED_CHARS = str(
    string.ascii_lowercase
    + string.ascii_uppercase
    + string.digits
    + string.punctuation,
)

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
