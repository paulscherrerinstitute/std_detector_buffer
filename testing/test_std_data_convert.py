import pytest

from testing.execution_helpers import executable, run_command, build_command


def test_converter_should_return_without_needed_arguments():
    command = f'{executable()}'
    rc, s, e = run_command(command)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')


@pytest.mark.skip("ongoing developments")
def test_converter():
    command = build_command(detector_json_filename='detector.json',
                            gains_and_pedestals='something.h5',
                            module_id=1)

    rc, s, e = run_command(command)

    print(s)
    print(e)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')
