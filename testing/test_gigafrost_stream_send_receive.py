import asyncio
from ctypes import Structure, c_uint64, c_uint16
import numpy as np
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigConverter, GigafrostConfigUdp, get_converter_buffer_data
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


@pytest.mark.asyncio
async def test_send_receive_stream(test_path):
    send_gf0 = build_command('std_stream_send_gf', test_path / 'gigafrost_detector.json', "tcp://127.0.0.1:50001", "0")
    send_gf1 = build_command('std_stream_send_gf', test_path / 'gigafrost_detector.json', "tcp://127.0.0.1:50002", "1")
    receive_fg = build_command('std_stream_receive_gf', test_path / 'gigafrost_detector_2.json',
                               "tcp://127.0.0.1:50001", "tcp://127.0.0.1:50002")

    slot = 3
    ctx = zmq.asyncio.Context()

    gf_config = GigafrostConfigConverter
    gf_config.socket_name = 'GF2-image'
    gf_config.name = 'GF2-image'

    with start_publisher_communication(ctx, gf_config) as (input_buffer, pub_socket):
        with run_command_in_parallel(send_gf0), run_command_in_parallel(send_gf1), run_command_in_parallel(receive_fg):
            gf_config.socket_name = 'GF22-image'
            gf_config.name = 'GF22-image'
            with start_subscriber_communication(ctx, gf_config) as (output_buffer, sub_socket):
                sent_data = get_converter_buffer_data(input_buffer, slot)
                for i in range(2):
                    index_start = int(i * len(sent_data) / 2)
                    sent_data[index_start] = 500 + i
                    sent_data[index_start + 1] = 600 + i

                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)
                assert np.frombuffer(msg, dtype='i8', count=1)[0] == slot

                data = get_converter_buffer_data(output_buffer, slot)
                start_index = int(GigafrostConfigUdp.image_pixel_height * GigafrostConfigUdp.image_pixel_width / 2)
                assert data[0] == 500
                assert data[1] == 600
                assert data[start_index] == 501
                assert data[start_index + 1] == 601
