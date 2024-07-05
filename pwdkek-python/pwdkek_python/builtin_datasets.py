import gzip
from enum import Enum
from io import BytesIO
from tempfile import NamedTemporaryFile
from typing import NamedTuple
from pathlib import Path
from urllib.request import urlopen, Request


DATASET_ROOT = Path(__file__).parent.parent / "datasets"


class BuiltinDatasetInfo(NamedTuple):
    file_name: str
    source_url: str

    @property
    def path(self) -> Path:
        file_path = DATASET_ROOT / self.file_name
        if not file_path.exists():
            self.download()
        return file_path

    @staticmethod
    def _remove_non_utf8(file_path: str) -> None:
        with open(file_path, "rb") as file:
            data = file.read()
        with open(file_path, "wb") as file:
            file.write(data.decode("utf-8", errors="ignore").encode("utf-8"))

    def download(self) -> None:
        from pwdkek_python.prepare_dataset import prepare_dataset

        DATASET_ROOT.mkdir(exist_ok=True)

        print("Downloading", self.source_url)
        file_data = urlopen(
            Request(self.source_url, headers={"User-Agent": "curl/8.3.0"})
        )
        decompressed_file = NamedTemporaryFile(delete=False)
        with decompressed_file as tmp_file:
            with gzip.open(BytesIO(file_data.read()), "rb") as file:
                tmp_file.write(file.read())

        print("Fixing encoding...")
        self._remove_non_utf8(decompressed_file.name)

        print("Preparing dataset...")
        prepare_dataset(decompressed_file.name, DATASET_ROOT / self.file_name)
        decompressed_file.close()
        print("Done!")


class BuiltInDataset(Enum):
    SMALL = BuiltinDatasetInfo(
        "rockyou-utf8-filtered-sorted.txt.gz",
        "https://raw.githubusercontent.com/zacheller/rockyou/master/rockyou.txt.tar.gz",
    )
    BIG = BuiltinDatasetInfo(
        "crackstation-human-only-utf8-filtered-sorted.txt.gz",
        "http://download.g0tmi1k.com/wordlists/large/crackstation-human-only.txt.gz",
    )

    @classmethod
    def names(cls):
        return [
            name.lower()
            for name, value in vars(cls).items()
            if isinstance(value, BuiltInDataset)
        ]


if __name__ == "__main__":
    print("Available datasets:", BuiltInDataset.names())
    assert BuiltInDataset.SMALL.value.path.exists()
    assert BuiltInDataset.BIG.value.path.exists()
