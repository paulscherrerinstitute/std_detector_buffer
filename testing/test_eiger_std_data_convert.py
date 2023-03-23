import functools
import time
from pathlib import Path

import numpy as np
import pytest
import zmq
import zmq.asyncio

from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_pull_communication
from testing.execution_helpers import build_command, run_command_in_parallel, send_receive
from std_buffer.eiger.data import EigerConfigUdp, EigerConfigConverter


def build_eiger_converter_command_full(test_path: Path, detector_file: str, module_id: int) -> str:
    return build_command('std_data_convert_eg', test_path / detector_file, module_id)


def get_udp_packet_array(input_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * EigerConfigUdp.data_bytes_per_packet
    data_of_slot = input_buffer[slot_start:slot_start + EigerConfigUdp.data_bytes_per_packet]
    return np.ndarray((int(EigerConfigUdp.data_bytes_per_packet / 2),), dtype='i2', buffer=data_of_slot)


def get_converter_packet_array(output_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * EigerConfigConverter.data_bytes_per_packet
    data_of_slot = output_buffer[slot_start:slot_start + EigerConfigConverter.data_bytes_per_packet]
    return np.ndarray((int(EigerConfigConverter.data_bytes_per_packet / 2),), dtype='i2',
                      buffer=data_of_slot)


@pytest.mark.asyncio
async def test_converter_1m_for_eiger(test_path):
    slot = 3
    build_eiger_converter_command = functools.partial(build_eiger_converter_command_full, test_path,
                                                      'eiger_detector_1M.json')
    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, EigerConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(build_eiger_converter_command(0)):
            with start_pull_communication(ctx, EigerConfigConverter) as (output_buffer, pull_socket):
                # fill data array with incremented data
                sent_data = get_udp_packet_array(input_buffer, slot)
                for i in range(256):
                    for j in range(256):
                        sent_data[i * 512 + j] = i
                    for j in range(256, 512):
                        sent_data[i * 512 + j] = i + 100

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8')[0] == slot
                output_data = get_converter_packet_array(output_buffer, slot)

                for i in range(256):
                    for j in range(256):
                        assert output_data[i*1030+j] == i
                    for j in range(258,514):
                        assert output_data[i*1030+j] == i + 100
