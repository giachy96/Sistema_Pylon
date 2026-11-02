import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
os.environ.setdefault("PYLON_NO_TTS", "1")

import pytest

SAMPLE = Path(os.environ.get("PYLON_SAMPLE_XLSM", Path(__file__).parent / "data" / "sample.xlsm"))


@pytest.fixture(scope="session")
def sample_path():
    if not SAMPLE.exists():
        pytest.skip("file xlsm di esempio non presente")
    return SAMPLE
