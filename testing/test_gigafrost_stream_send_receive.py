import asyncio
from ctypes import Structure, c_uint64, c_uint16
import numpy as np
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigConverter, GigafrostConfigUdp
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import build_command, run_command_in_parallel, send_receive


class GFFrame(Structure):
    _pack_ = 1
    _fields_ = [("id", c_uint64),
                ("height", c_uint64),
                ("width", c_uint64),
                ("dtype", c_uint16),
                ("status", c_uint16),
                ("source_id", c_uint16)]

    def __str__(self):
        return f"{self.id=}"


def get_converter_buffer_data(buffer, slot):
    slot_start = slot * GigafrostConfigConverter.bytes_per_packet + GigafrostConfigConverter.meta_bytes_per_packet
    data_of_slot = buffer[slot_start:slot_start + GigafrostConfigConverter.data_bytes_per_packet]
    return np.ndarray((int(GigafrostConfigConverter.data_bytes_per_packet / 2),), dtype='u2',
                      buffer=data_of_slot)


@pytest.mark.asyncio
async def test_send_receive_stream_for_multiple_packages(test_path):
    send_gf0 = build_command('std_stream_send_gf', test_path / 'gigafrost_detector.json', "tcp://127.0.0.1:50001", "0")
    send_gf1 = build_command('std_stream_send_gf', test_path / 'gigafrost_detector.json', "tcp://127.0.0.1:50002", "1")
    receive_fg = build_command('std_stream_receive_gf', test_path / 'gigafrost_detector_2.json',
                               "tcp://127.0.0.1:50001", "tcp://127.0.0.1:50002")

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, GigafrostConfigConverter) as (input_buffer, pub_socket):
        with run_command_in_parallel(send_gf0), run_command_in_parallel(send_gf1), run_command_in_parallel(receive_fg):
            subscriber_config = GigafrostConfigConverter
            subscriber_config.name = 'GF22-sync'
            with start_subscriber_communication(ctx, subscriber_config) as (output_buffer, sub_socket):
                for slot in range(20):
                    sent_data = get_converter_buffer_data(input_buffer, slot)
                    for i in range(2):
                        index_start = int(i * len(sent_data) / 2)
                        sent_data[index_start] = 500 + i + slot
                        sent_data[index_start + 1] = 600 + i + slot

                    msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)
                    assert np.frombuffer(msg, dtype='i8', count=1)[0] == slot

                    data = get_converter_buffer_data(output_buffer, slot)
                    start_index = int(GigafrostConfigUdp.image_pixel_height * GigafrostConfigUdp.image_pixel_width / 2)
                    assert data[0] == 500 + slot
                    assert data[1] == 600 + slot
                    assert data[start_index] == 501 + slot
                    assert data[start_index + 1] == 601 + slot
