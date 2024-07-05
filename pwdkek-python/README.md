# Pwd Kek

Ckek how strong is your password

## Installation
```shell
pip install pwdkek-python
```

## Usage
```
$ python3 -m pwdkek-python --help

usage: __main__.py [-h] [--dataset DATASET]

Password Complexity Estimator

options:
  -h, --help         show this help message and exit
  --dataset DATASET  Built-in dataset name or path to the dataset file
```

## Output example
```
$ python -m pwdkek-python
Loading...

Enter a password: test
Password entropy: 14.381098099594738
Time to decode with 1Gh/s: 0 years 0 days 0 hours 0 minutes 0 seconds
Tier: Pathetic

Enter a password: reallyextremepassword!!!!?098
Password entropy: 70.5169500105413
Time to decode with 1Gh/s: 535686 years 113 days 17 hours 32 minutes 46 seconds
Tier: Extreme

Enter a password: ^C
```
