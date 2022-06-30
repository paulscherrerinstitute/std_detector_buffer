import time

import pytest

from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel


def test_converter_should_return_without_needed_arguments():
    command = f'{executable()}'
    rc, s, e = run_command(command)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')


def test_converter_startup_shutdown():
    command = build_command(detector_json_filename='detector.json',
                            gains_and_pedestals='gains_1_pedestals_0.h5',
                            module_id=1)

    with run_command_in_parallel(command):
        time.sleep(20)

    assert True
