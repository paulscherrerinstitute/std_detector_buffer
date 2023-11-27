import time

import numpy as np
import pytest
import zmq
import zmq.asyncio

from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_pull_communication
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel, send_receive, \
    get_array
from std_buffer.jungfrau.data import JungfrauConfigUdp, JungfrauConfigConverter


def build_jungfrau_converter_command(test_path, pedestals='gains_1_pedestals_0.h5') -> str:
    return build_command('std_data_convert_jf', test_path / 'jungfrau_detector.json', '-g', test_path / pedestals,
                         JungfrauConfigUdp().id)


def test_converter_should_return_without_needed_arguments():
    command = f'{executable(name="std_data_convert_jf")}'
    rc, s, e = run_command(command)

    assert rc == 1
    assert len(e) > 0
    assert 'Usage: std_data_convert_jf' in e


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    slot = 3
    command = build_jungfrau_converter_command(test_path)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp()) as (input_buffer, pub_socket):
        with run_command_in_parallel(command, 3):
            with start_pull_communication(ctx, JungfrauConfigConverter()) as (output_buffer, pull_socket):
                # fill data array with incremented data
                sent_data = get_array(input_buffer, slot, 'i2', JungfrauConfigUdp())
                for i in range(len(sent_data)):
                    sent_data[i] = i % 1000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8')[0] == slot
                output_data = get_array(output_buffer, slot, 'f4', JungfrauConfigConverter())
                for i in range(512):
                    for j in range(1024):
                        assert output_data[i * 2048 + j] == sent_data[i * 1024 + j]


@pytest.mark.asyncio
async def test_converter_modifying_image_with_gains_and_pedestals(test_path):
    slot = 7
    command = build_jungfrau_converter_command(test_path, 'gains_2_pedestals_minus1.h5')

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp()) as (input_buffer, pub_socket):
        with run_command_in_parallel(command, 3):
            with start_pull_communication(ctx, JungfrauConfigConverter()) as (output_buffer, pull_socket):
                # fill data array with incremented data
                sent_data = get_array(input_buffer, slot, 'i2', JungfrauConfigUdp())
                for i in range(len(sent_data)):
                    sent_data[i] = i % 1000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=pull_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8')[0] == slot
                output_data = get_array(output_buffer, slot, 'f4', JungfrauConfigConverter())
                for i in range(512):
                    for j in range(1024):
                        assert output_data[i * 2048 + j] == (sent_data[i * 1024 + j] + 1) * 2
