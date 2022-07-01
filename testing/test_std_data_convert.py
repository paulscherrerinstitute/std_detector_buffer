import time

import numpy as np
import zmq

from testing.communication import open_shared_memory
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel


def test_converter_should_return_without_needed_arguments():
    command = f'{executable()}'
    rc, s, e = run_command(command)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')


def test_converter_startup_shutdown():
    module_id = 1
    command = build_command(detector_json_filename='detector.json',
                            gains_and_pedestals='gains_1_pedestals_0.h5',
                            module_id=module_id)

    ctx = zmq.Context()
    zmq_send_socket = ctx.socket(zmq.PUB)
    zmq_send_socket.bind(f'ipc:///tmp/std-daq-jungfrau1:{50020 + module_id}')

    with open_shared_memory(name=f'jungfrau{module_id}', create=True, size=512 * 1024 * 2) as input_buffer:
        with run_command_in_parallel(command):
            time.sleep(1)
            with open_shared_memory(name=f'jungfrau{module_id}-converted', size=512 * 1024 * 4) as output_buffer:
                sent_data = np.ndarray((5,), dtype='b', buffer=input_buffer)
                sent_data[:] = np.frombuffer(b'hello', dtype='b')
                zmq_send_socket.send(np.array([0], dtype='i8'))

                time.sleep(0.1)
                received_data = np.ndarray((5,), dtype='b', buffer=output_buffer)

                assert np.array_equal(received_data, sent_data)

    assert True
