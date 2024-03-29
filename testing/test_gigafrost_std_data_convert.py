import numpy as np
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigUdp, GigafrostConfigConverter
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_pull_communication
from testing.execution_helpers import build_command, run_command_in_parallel, send_receive, get_array


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    slot = 3
    command = build_command('std_data_convert_gf', test_path / 'gigafrost_detector.json', GigafrostConfigUdp().id)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigUdp()) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_pull_communication(ctx, GigafrostConfigConverter()) as (output_buffer, pull_socket):
                sent_data = get_array(input_buffer, slot, 'i1', GigafrostConfigUdp())
                # 513, 514, 515, 516 - 4 pixels
                sent_data[0] = 0b00000001
                sent_data[1] = 0b00100010
                sent_data[2] = 0b00100000
                sent_data[3] = 0b00000011
                sent_data[4] = 0b01000010
                sent_data[5] = 0b00100000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)
                assert np.frombuffer(msg, dtype='u2')[0] == slot
                data = get_array(output_buffer, slot, 'u2', GigafrostConfigConverter())
                start_index = int(GigafrostConfigUdp().image_pixel_height * GigafrostConfigUdp().image_pixel_width / 2)
                assert data[start_index] == 513
                assert data[start_index + 1] == 514
                assert data[start_index + 2] == 515
                assert data[start_index + 3] == 516
