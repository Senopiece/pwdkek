import bisect
from dataclasses import dataclass
from datetime import timedelta
from enum import Enum
import gzip
from math import log2
import string
from pathlib import Path

from pwdkek_python.builtin_datasets import BuiltInDataset

PASSWORD_ALLOWED_CHARS = str(
    string.ascii_lowercase
    + string.ascii_uppercase
    + string.digits
    + string.punctuation,
)


class PasswordComplexityTiers(Enum):
    PATHETIC = "Pathetic"  # a few days
    LOW = "Low"  # a month
    MEDIUM = "Medium"  # a year
    HIGH = "High"  # 10 years
    EXTREME = "Extreme"  # 100+ years


@dataclass
class PasswordComplexityEstimate:
    entropy: float
    ttd: timedelta
    tier: PasswordComplexityTiers


class PasswordComplexityEstimator:
    def __init__(
        self,
        dataset_path: str | Path | BuiltInDataset,
    ):
        if isinstance(dataset_path, BuiltInDataset):
            dataset_path = dataset_path.value.path

        with gzip.open(dataset_path) as file:
            self._passwords = [line.decode() for line in file.readlines()]

        if len(self._passwords) == 0:
            raise ValueError(
                "something is wrong with the dataset, maybe file not found"
            )

    def _count_prefixed(self, prefix: str):
        start = bisect.bisect_left(self._passwords, prefix)
        end = bisect.bisect_right(self._passwords, prefix + "z" * 100)
        return end - start

    def _probability(self, given_prefix: str, expecting_next_char: str):
        total = self._count_prefixed(given_prefix)
        matched = self._count_prefixed(given_prefix + expecting_next_char)
        if total == 0:
            return 0
        return matched / total

    def _entropy(self, password: str):
        if any(char not in PASSWORD_ALLOWED_CHARS for char in password):
            raise ValueError("Password contains invalid characters")

        prefix = ""
        entropy = 0

        for token in password:
            while True:
                chosen_token_prob = self._probability(prefix, token)

                if chosen_token_prob == 0:
                    if prefix == "":
                        chosen_token_prob = 0.000000001  # A Ultra Extreme tier for sure
                        break
                    prefix = prefix[1:]
                else:
                    break

            entropy += -log2(chosen_token_prob)
            prefix += token

        return entropy

    @classmethod
    def _ttd(cls, entropy: float):
        return timedelta(seconds=10e-9 * pow(2, entropy))

    @classmethod
    def _tier(cls, ttd: timedelta):
        COMPLEXITIES = list(PasswordComplexityTiers)

        for i, tier in enumerate(COMPLEXITIES[:-1]):
            threshold = timedelta(days=365 * pow(10, i - 2))

            if ttd < threshold:
                return tier

        return COMPLEXITIES[-1]

    def estimate(self, password: str):
        entropy = self._entropy(password)

        ttd = timedelta.max
        try:
            ttd = self._ttd(entropy)
        except OverflowError:
            pass

        tier = self._tier(ttd)

        return PasswordComplexityEstimate(
            entropy,
            ttd,
            tier,
        )
