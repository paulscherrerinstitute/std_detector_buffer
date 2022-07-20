from pathlib import Path

import pytest


@pytest.fixture
def test_path():
    return Path(__file__).parent.absolute() / 'test_files'
