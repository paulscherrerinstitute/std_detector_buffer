import time

import numpy as np
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigUdp, GigafrostConfigConverter
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel, push_to_buffer, \
    send_receive
from std_buffer.jungfrau.data import JungfrauConfigUdp, JungfrauConfigConverter


def get_udp_packet_array(input_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * GigafrostConfigUdp.bytes_per_packet + GigafrostConfigUdp.meta_bytes_per_packet
    data_of_slot = input_buffer[slot_start:slot_start + GigafrostConfigUdp.data_bytes_per_packet]
    return np.ndarray((int(GigafrostConfigUdp.data_bytes_per_packet / 2),), dtype='i2', buffer=data_of_slot)


@pytest.mark.asyncio
async def test_converter_send_simple_data_for_packet_with_0_id(test_path):
    command = build_command('std_data_convert_gf', test_path / 'gigafrost_detector.json',
                            GigafrostConfigUdp.quadrant_id,
                            GigafrostConfigUdp.id)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_subscriber_communication(ctx, GigafrostConfigConverter) as (output_buffer, sub_socket):
                sent_data = push_to_buffer(input_buffer, b'hello')
                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=0)

                assert np.frombuffer(msg, dtype='i8') == 0
                received_data = np.ndarray((5,), dtype='b', buffer=output_buffer)

                assert np.array_equal(received_data, sent_data)


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    slot = 3
    command = build_command('std_data_convert_gf', test_path / 'gigafrost_detector.json',
                            GigafrostConfigUdp.quadrant_id,
                            GigafrostConfigUdp.id)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_subscriber_communication(ctx, GigafrostConfigConverter) as (output_buffer, sub_socket):
                # fill data array with incremented data
                sent_data = get_udp_packet_array(input_buffer, slot)
                for i in range(len(sent_data)):
                    sent_data[i] = (i+1) % 1000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8') == slot
                # assert np.array_equal(get_converter_packet_array(output_buffer, slot), sent_data)
