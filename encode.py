from dataclasses import dataclass
import string
from typing import IO, Dict

from tqdm import tqdm


rockyou_path = "/home/nabuki/datasets/rockyou.txt"

# Define the character set used in passwords
characters = str(
    string.ascii_lowercase
    + string.ascii_uppercase
    + string.digits
    + string.punctuation,
)
char_to_index = {char: idx for idx, char in enumerate(characters)}


def tokenize(s: str):
    return (char_to_index[ch] for ch in s if ch in characters)


@dataclass
class Node:
    _forward: Dict[int, "Node"]
    _visits: int = 0

    def __init__(self):
        self._forward = {}

    def visit(self, token: int):
        e = self._forward.get(token)
        if e is None:
            e = Node()
            self._forward[token] = e
        e._visits += 1
        return e

    def normalized_transition_weights(self):
        s = sum(node._visits for node in self._forward.values())

        if s == 0:
            return None

        res = [0.0] * len(characters)
        for k, v in self._forward.items():
            res[k] = v._visits / s

        return res

    def write_normalized(self, sink: IO[bytes]):
        transitions = self.normalized_transition_weights()

        # write zeros if endpath
        if transitions is None:
            sink.write(b"\x00" * len(characters))
            return

        # quantize 0..1f to 0..255i
        transitions = [int(v * 255) for v in transitions]

        # write quantized transition probabilities (byte for each token)
        for v in transitions:
            sink.write(v.to_bytes())

        # for each non-zero probability transition, write them recursively
        nonzero_transitions_idexes = [
            index for index, value in enumerate(transitions) if value != 0
        ]
        for i in nonzero_transitions_idexes:
            self._forward[i].write_normalized(sink)


# Collect tree
root = Node()
with open(rockyou_path, "r", encoding="latin1") as file:
    for line in tqdm(file):
        curr = root
        for token in tokenize(line.strip()):
            curr = curr.visit(token)


# Write it as probabilities (normalized)
print("Writing...")
with open("res.pfx", "wb") as f:
    root.write_normalized(f)
