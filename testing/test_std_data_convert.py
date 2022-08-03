import time

import numpy as np
import pytest
import zmq
import zmq.asyncio

from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication
from testing.execution_helpers import executable, run_command, build_command, run_command_in_parallel
from testing.jungfrau.data import JungfrauConfigUdp, JungfrauConfigConverter


def get_udp_packet_array(input_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * JungfrauConfigUdp.bytes_per_packet + JungfrauConfigUdp.meta_bytes_per_packet
    data_of_slot = input_buffer[slot_start:slot_start + JungfrauConfigUdp.data_bytes_per_packet]
    return np.ndarray((int(JungfrauConfigUdp.data_bytes_per_packet / 2),), dtype='i2', buffer=data_of_slot)


def get_converter_packet_array(output_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * JungfrauConfigConverter.bytes_per_packet + JungfrauConfigConverter.meta_bytes_per_packet
    data_of_slot = output_buffer[slot_start:slot_start + JungfrauConfigConverter.data_bytes_per_packet]
    return np.ndarray((int(JungfrauConfigConverter.data_bytes_per_packet / 4),), dtype='f4',
                      buffer=data_of_slot)


async def send_receive(pub_socket: zmq.asyncio.Socket, slot: int, sub_socket: zmq.asyncio.Socket):
    msg = sub_socket.recv()
    time.sleep(0.1)
    await pub_socket.send(np.array([slot], dtype='i8'))
    time.sleep(0.1)
    return await msg


def push_to_buffer(input_buffer, data):
    sent_data = np.ndarray((len(data),), dtype='b', buffer=input_buffer)
    sent_data[:] = np.frombuffer(data, dtype='b')
    return sent_data


def build_jungfrau_converter_command(test_path, pedestals='gains_1_pedestals_0.h5') -> str:
    return build_command('std_data_convert', test_path / 'jungfrau_detector.json', test_path / pedestals,
                         JungfrauConfigUdp.id)


def test_converter_should_return_without_needed_arguments():
    command = f'{executable(name="std_data_convert")}'
    rc, s, e = run_command(command)

    assert rc == 255
    assert len(e) == 0
    assert s.startswith('Usage: std_data_convert')


@pytest.mark.asyncio
async def test_converter_send_simple_data_for_packet_with_0_id(test_path):
    command = build_jungfrau_converter_command(test_path)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_subscriber_communication(ctx, JungfrauConfigConverter) as (output_buffer, sub_socket):
                # send msg and await reply from converter
                sent_data = push_to_buffer(input_buffer, b'hello')
                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=0)

                # test reply content and (not) modified buffer
                assert np.frombuffer(msg, dtype='i8') == 0
                received_data = np.ndarray((5,), dtype='b', buffer=output_buffer)

                assert np.array_equal(received_data, sent_data)


@pytest.mark.asyncio
async def test_converter_send_real_image_with_custom_slot(test_path):
    slot = 3
    command = build_jungfrau_converter_command(test_path)

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_subscriber_communication(ctx, JungfrauConfigConverter) as (output_buffer, sub_socket):
                # fill data array with incremented data
                sent_data = get_udp_packet_array(input_buffer, slot)
                for i in range(len(sent_data)):
                    sent_data[i] = i % 1000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8') == slot
                assert np.array_equal(get_converter_packet_array(output_buffer, slot), sent_data)


@pytest.mark.asyncio
async def test_converter_modifying_image_with_gains_and_pedestals(test_path):
    slot = 7
    command = build_jungfrau_converter_command(test_path, 'gains_2_pedestals_minus1.h5')

    ctx = zmq.asyncio.Context()

    with start_publisher_communication(ctx, JungfrauConfigUdp) as (input_buffer, pub_socket):
        with run_command_in_parallel(command):
            with start_subscriber_communication(ctx, JungfrauConfigConverter) as (output_buffer, sub_socket):
                # fill data array with incremented data
                sent_data = get_udp_packet_array(input_buffer, slot)
                for i in range(len(sent_data)):
                    sent_data[i] = i % 1000

                msg = await send_receive(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)

                assert np.frombuffer(msg, dtype='i8') == slot
                converted_data = get_converter_packet_array(output_buffer, slot)
                for i in range(len(converted_data)):
                    assert converted_data[i] == (i % 1000 + 1) * 2
