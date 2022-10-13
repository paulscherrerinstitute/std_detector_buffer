import asyncio
import time

import numpy as np
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigUdp, GigafrostConfigConverter
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, \
    start_pull_communication
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel, push_to_buffer, \
    send_receive


def get_udp_packet_array(input_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * GigafrostConfigUdp.bytes_per_packet + GigafrostConfigUdp.meta_bytes_per_packet
    data_of_slot = input_buffer[slot_start:slot_start + GigafrostConfigUdp.data_bytes_per_packet]
    return np.ndarray((int(GigafrostConfigUdp.data_bytes_per_packet),), dtype='i1', buffer=data_of_slot)


def get_converter_buffer_data(output_buffer, slot):
    slot_start = slot * GigafrostConfigConverter.bytes_per_packet + GigafrostConfigConverter.meta_bytes_per_packet
    data_of_slot = output_buffer[slot_start:slot_start + GigafrostConfigConverter.data_bytes_per_packet]
    return np.ndarray((int(GigafrostConfigConverter.data_bytes_per_packet / 2),), dtype='u2',
                      buffer=data_of_slot)


@pytest.mark.asyncio
async def test_converter_send_simple_data_for_packet_with_0_id(test_path):
    command = build_command('std_data_convert_gf', test_path / 'gigafrost_detector.json', GigafrostConfigUdp.id)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_pull_communication(ctx, GigafrostConfigConverter) as (output_buffer, pull_socket):
                sent_data = push_to_buffer(input_buffer, b'hello')
                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=0)

                assert np.frombuffer(msg, dtype='i8') == 0
                received_data = np.ndarray((5,), dtype='b', buffer=output_buffer)

                assert np.array_equal(received_data, sent_data)


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    slot = 3
    command = build_command('std_data_convert_gf', test_path / 'gigafrost_detector.json', GigafrostConfigUdp.id)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            # time.sleep(2)
            with start_pull_communication(ctx, GigafrostConfigConverter) as (output_buffer, pull_socket):
                sent_data = get_udp_packet_array(input_buffer, slot)
                # 513, 514, 515, 516 - 4 pixels
                sent_data[0] = 0b00000001
                sent_data[1] = 0b00100010
                sent_data[2] = 0b00100000
                sent_data[3] = 0b00000011
                sent_data[4] = 0b01000010
                sent_data[5] = 0b00100000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8') == slot
                data = get_converter_buffer_data(output_buffer, slot)
                start_index = int(GigafrostConfigUdp.image_pixel_height * GigafrostConfigUdp.image_pixel_width / 2)
                assert data[start_index] == 513
                assert data[start_index + 1] == 514
                assert data[start_index + 2] == 515
                assert data[start_index + 3] == 516
