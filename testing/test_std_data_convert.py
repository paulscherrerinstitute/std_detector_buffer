import time

import numpy as np
import zmq

from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel


class JungfrauConfigUdp:
    id = 1
    name = f'jungfrau{id}'
    buffer_size = 512 * 1024 * 2
    udp_port_base = 50020


class JungfrauConfigConverter:
    id = 1
    name = f'jungfrau{id}-converted'
    buffer_size = 512 * 1024 * 4
    udp_port_base = 50020


def push_to_buffer(input_buffer, data):
    sent_data = np.ndarray((5,), dtype='b', buffer=input_buffer)
    sent_data[:] = np.frombuffer(data, dtype='b')
    return sent_data


def test_converter_should_return_without_needed_arguments():
    command = f'{executable()}'
    rc, s, e = run_command(command)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')


def test_converter_send_simple_data_for_packet_with_0_id():
    command = build_command(detector_json_filename='detector.json',
                            gains_and_pedestals='gains_1_pedestals_0.h5',
                            module_id=JungfrauConfigUdp.id)

    ctx = zmq.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            time.sleep(1)  # time for the std_data_convert executable to startup
            with start_subscriber_communication(ctx, JungfrauConfigConverter) as (output_buffer, sub_socket):
                sent_data = push_to_buffer(input_buffer, b'hello')
                pub_socket.send(np.array([0], dtype='i8'))

                msg = sub_socket.recv()
                assert np.frombuffer(msg, dtype='i8') == 0
                received_data = np.ndarray((5,), dtype='b', buffer=output_buffer)

                assert np.array_equal(received_data, sent_data)
