import asyncio
import time

import bitshuffle
import pytest
import zmq
import zmq.asyncio
import numpy as np
import copy

from std_buffer.gigafrost.data import GigafrostConfigConverter, get_converter_buffer_data
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import build_command, run_command_in_parallel
import std_buffer.image_metadata_pb2 as daq_proto


@pytest.mark.asyncio
@pytest.mark.skip(reason="Maciej help me staph!")
async def test_compression(test_path):
    compression_cmd = build_command('std_data_compress', test_path / 'gigafrost_detector.json', '-t', '4')

    ctx = zmq.asyncio.Context()

    metadata = daq_proto.ImageMetadata()
    metadata.image_id = 1
    metadata.width = 2016
    metadata.height = 2016
    metadata.dtype = daq_proto.ImageMetadataDtype.uint16

    config = copy.copy(GigafrostConfigConverter)
    config.socket_name = 'GF2-image'

    with start_publisher_communication(ctx, config) as (input_buffer, pub_socket):
        with run_command_in_parallel(compression_cmd):
            config.socket_name = 'GF2-compressed'
            config.name = 'GF2-compressed'
            with start_subscriber_communication(ctx, config) as (output_buffer, sub_socket):
                uncompressed_data = get_converter_buffer_data(input_buffer, metadata.image_id)
                for i in range(len(uncompressed_data)):
                    uncompressed_data[i] = i % 256

                await send_receive(metadata, pub_socket, sub_socket)

                assert metadata.image_id == 1
                assert metadata.status == daq_proto.ImageMetadataStatus.compressed_image

                decoded_data = await get_decoded_data(metadata, output_buffer)


                print(uncompressed_data)
                print(decoded_data)
                for i in range(len(uncompressed_data)):
                    assert uncompressed_data[i] == decoded_data[i]

                del uncompressed_data
                del decoded_data


async def get_decoded_data(metadata, output_buffer):
    slot_start = metadata.image_id * GigafrostConfigConverter.data_bytes_per_packet
    compressed_data = output_buffer[slot_start:slot_start + GigafrostConfigConverter.data_bytes_per_packet]
    data = np.frombuffer(compressed_data, dtype=np.uint16)
    return bitshuffle.decompress_lz4(data, (int(GigafrostConfigConverter.data_bytes_per_packet/2),), np.dtype('u2'))


async def send_receive(metadata, pub_socket, sub_socket):
    msg = sub_socket.recv()
    time.sleep(0.1)
    await pub_socket.send(metadata.SerializeToString())
    time.sleep(0.1)
    msg = await msg
    metadata.ParseFromString(msg)
