import pytest as pytest

from testing.execution_helpers import build_command, run_command_in_parallel
from testing.jungfrau.std_stream_recv_binary import StdStreamRecvBinary


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    command = build_command('std_stream_send_binary', test_path / 'jungfrau_detector.json', 'ipc://send_binary_test')

    with run_command_in_parallel(command):
        with StdStreamRecvBinary('ipc://send_binary_test') as input_stream:
            pass
