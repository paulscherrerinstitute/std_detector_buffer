import asyncio
import time
# import blosc2
import pytest
import zmq
import zmq.asyncio
import numpy as np

from std_buffer.gigafrost.data import GigafrostConfigConverter
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import build_command, run_command_in_parallel, get_array
import std_buffer.image_metadata_pb2 as daq_proto


@pytest.mark.asyncio
@pytest.mark.skip(reason="blosc2 import doesn't install correctly argh - check it!")
async def test_compression(test_path):
    compression_cmd = build_command('std_data_compress_blosc2', test_path / 'gigafrost_detector.json', '-t', '4')

    ctx = zmq.asyncio.Context()

    metadata = daq_proto.ImageMetadata()
    metadata.image_id = 3
    metadata.width = 2016
    metadata.height = 2016
    metadata.dtype = daq_proto.ImageMetadataDtype.uint16

    config = GigafrostConfigConverter()
    config.socket_name = 'GF2-image'

    with start_publisher_communication(ctx, config) as (input_buffer, pub_socket):
        with run_command_in_parallel(compression_cmd):
            config.socket_name = 'GF2-blosc2'
            config.name = 'GF2-blosc2'
            with start_subscriber_communication(ctx, config) as (output_buffer, sub_socket):
                uncompressed_data = get_array(input_buffer, metadata.image_id, 'i2', config)
                for i in range(len(uncompressed_data)):
                    uncompressed_data[i] = i % 256

                await send_receive(metadata, pub_socket, sub_socket)

                assert metadata.image_id == 3
                assert metadata.compression == daq_proto.ImageMetadataCompression.blosc2
                decoded_data = await get_decoded_data(metadata, output_buffer)

                for i in range(len(uncompressed_data)):
                    assert uncompressed_data[i] == decoded_data[i]

                del uncompressed_data
                del decoded_data


async def get_decoded_data(metadata, output_buffer):
    packet_size = GigafrostConfigConverter().data_bytes_per_packet
    slot_start = metadata.image_id * packet_size
    compressed_data = output_buffer[slot_start:slot_start + packet_size]
    decompressed_data = blosc2.decompress(compressed_data)
    return np.ndarray((int(packet_size / 2),), dtype='u2', buffer=decompressed_data)


async def send_receive(metadata, pub_socket, sub_socket):
    msg = sub_socket.recv()
    time.sleep(0.1)
    await pub_socket.send(metadata.SerializeToString())
    time.sleep(0.1)
    msg = await msg
    metadata.ParseFromString(msg)
