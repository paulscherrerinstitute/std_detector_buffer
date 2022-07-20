from pathlib import Path

import pytest


@pytest.fixture
def test_path():
    return Path(__file__).parent.absolute() / 'test_files'


@pytest.fixture
def cleanup_jungfrau_shared_memory():
    yield
    for file in Path('/dev/shm').glob('jungfrau*'):
        file.unlink()
