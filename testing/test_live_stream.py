import asyncio
import time
from ctypes import Structure, c_uint64, c_uint16
import pytest
import zmq
import zmq.asyncio

from std_buffer.gigafrost.data import GigafrostConfigConverter, get_converter_buffer_data
from testing.fixtures import test_path
from testing.communication import start_publisher_communication
from testing.execution_helpers import build_command, run_command_in_parallel
import testing.std_daq.image_metadata_pb2 as daq_proto


@pytest.mark.asyncio
async def test_send_live_stream(test_path):
    live_stream_cmd = build_command('std_live_stream', test_path / 'gigafrost_detector.json', 'tcp://127.0.0.1:50001',
                                    '5')

    ctx = zmq.asyncio.Context()

    metadata = daq_proto.ImageMetadata()
    metadata.image_id = 3

    config = GigafrostConfigConverter
    config.socket_name = 'GF2-image'

    with start_publisher_communication(ctx, config) as (input_buffer, pub_socket):
        with run_command_in_parallel(live_stream_cmd):
            sub_socket = ctx.socket(zmq.SUB)
            sub_socket.connect(f'tcp://127.0.0.1:50001')
            sub_socket.subscribe('')

            msg = sub_socket.recv_multipart()
            time.sleep(0.1)
            await pub_socket.send(metadata.SerializeToString())
            time.sleep(0.1)
            msg = await msg
            assert b'{"htype":"array-1.0", "shape":[2016,2016], "type":"uint16", "frame":3}' == msg[0]
