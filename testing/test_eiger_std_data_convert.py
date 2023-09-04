import functools
from pathlib import Path

import numpy as np
import pytest
import zmq
import zmq.asyncio

from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_pull_communication
from testing.execution_helpers import build_command, run_command_in_parallel, send_receive, get_array
from std_buffer.eiger.data import EigerConfigUdp, EigerConfigConverter


def build_eiger_converter_command_full(test_path: Path, detector_file: str, module_id: int) -> str:
    return build_command('std_data_convert_eg', test_path / detector_file, module_id)


def fill_data_to_send(input_buffer, slot):
    sent_data = get_array(input_buffer, slot, 'i2', EigerConfigUdp())
    for i in range(256):
        for j in range(256):
            sent_data[i * 512 + j] = i
        for j in range(256, 512):
            sent_data[i * 512 + j] = i + 100


@pytest.mark.asyncio
async def test_converter_half_m_for_eiger_first_quarter(test_path):
    slot = 3
    build_eiger_converter_command = functools.partial(build_eiger_converter_command_full, test_path,
                                                      'eiger_detector_0_5M.json')
    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, EigerConfigUdp()) as (input_buffer, pub_socket):
        with run_command_in_parallel(build_eiger_converter_command(0)):
            with start_pull_communication(ctx, EigerConfigConverter()) as (output_buffer, pull_socket):
                fill_data_to_send(input_buffer, slot)

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8')[0] == slot
                output_data = get_array(output_buffer, slot, 'i2', EigerConfigConverter())

                for i in range(256):
                    for j in range(256):
                        assert output_data[i * 1030 + j] == i
                    for j in range(258, 514):
                        assert output_data[i * 1030 + j] == i + 100


@pytest.mark.asyncio
async def test_converter_half_m_for_eiger_fourth_quarter(test_path):
    slot = 4
    build_eiger_converter_command = functools.partial(build_eiger_converter_command_full, test_path,
                                                      'eiger_detector_0_5M.json')
    ctx = zmq.asyncio.Context()
    config = EigerConfigUdp()
    config.name = 'EG05M-3'
    with start_publisher_communication(ctx, config) as (input_buffer, pub_socket):
        with run_command_in_parallel(build_eiger_converter_command(3)):
            config = EigerConfigConverter()
            config.converter_index = 3
            with start_pull_communication(ctx, config) as (output_buffer, pull_socket):
                fill_data_to_send(input_buffer, slot)

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8')[0] == slot
                output_data = get_array(output_buffer, slot, 'i2', EigerConfigConverter())
                for i in range(256):
                    for j in range(256):
                        jump = ((258 + i) * 1030) + 516 + j
                        assert output_data[jump] == 255 - i
                    for j in range(258, 514):
                        jump = ((258 + i) * 1030) + 516 + j
                        assert output_data[jump] == 255 - i + 100
