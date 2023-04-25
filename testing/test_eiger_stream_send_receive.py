import asyncio
import shlex
import subprocess
from ctypes import Structure, c_uint64, c_uint16
from pathlib import Path

import numpy as np
import pytest
import zmq
import zmq.asyncio

import testing.std_daq.image_metadata_pb2
from std_buffer.eiger.data import EigerConfigConverter, EigerConfigUdp, get_converter_buffer_data
from testing.fixtures import test_path
from testing.communication import start_publisher_communication, start_subscriber_communication, \
    start_pull_communication
from testing.execution_helpers import build_command, run_command_in_parallel, send_receive, send_receive_proto



class EGFrame(Structure):
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
async def test_sender_should_return_when_nothing_to_do(test_path):
    stream_send = build_command('std_stream_send', test_path / 'eiger_detector_0_5M.json',
                                "tcp://127.0.0.1:50001", "7")

    process_result = subprocess.run(args=(shlex.split(stream_send)),
                                    cwd=Path(__file__).parent,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)

    assert process_result.returncode == 0
    assert process_result.stdout == b''
    assert process_result.stderr == b''


@pytest.mark.asyncio
async def test_receiver_should_return_when_nothing_to_do(test_path):
    stream_receive = build_command('std_stream_receive', test_path / 'eiger_detector_0_5M.json',
                                   "tcp://127.0.0.1:50001", "7")

    process_result = subprocess.run(args=(shlex.split(stream_receive)),
                                    cwd=Path(__file__).parent,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)

    assert process_result.returncode == 0
    assert process_result.stdout == b''
    assert process_result.stderr == b''


@pytest.mark.asyncio
async def test_send_receive_stream(test_path):
    send_eg0 = build_command('std_stream_send', test_path / 'eiger_detector_0_5M.json', "tcp://127.0.0.1:50001", "0")
    send_eg1 = build_command('std_stream_send', test_path / 'eiger_detector_0_5M.json', "tcp://127.0.0.1:50002", "1")
    receive_eg0 = build_command('std_stream_receive', test_path / 'eiger_detector_0_5M.json',
                                "tcp://127.0.0.1:50001", "0")
    receive_eg1 = build_command('std_stream_receive', test_path / 'eiger_detector_0_5M.json',
                                "tcp://127.0.0.1:50002", "1")

    slot = 3
    ctx = zmq.asyncio.Context()

    eg_config = EigerConfigConverter
    eg_config.socket_name = 'EG-image'
    eg_config.name = 'EG-image'
    with start_publisher_communication(ctx, eg_config) as (input_buffer, pub_socket):
        with run_command_in_parallel(send_eg0), run_command_in_parallel(send_eg1), run_command_in_parallel(
                receive_eg0), run_command_in_parallel(receive_eg1):
            eg_config.socket_name = 'EG-sync'
            eg_config.name = 'EG-image'
            
            with start_pull_communication(ctx, eg_config) as (output_buffer, sub_socket):
                sent_data = get_converter_buffer_data(input_buffer, slot)
                for i in range(8):
                    index_start = int(i * len(sent_data) / 8)
                    sent_data[index_start] = 500 + i
                    sent_data[index_start + 1] = 600 + i

                msg = await send_receive_proto(pub_socket=pub_socket, sub_socket=sub_socket, slot=slot)
                assert msg.image_id == slot

                data = get_converter_buffer_data(output_buffer, slot)
                assert data[0] == 500
                assert data[1] == 600
                start_index = int(EigerConfigUdp.image_pixel_height * EigerConfigUdp.image_pixel_width / 8)
                assert data[start_index] == 501
                assert data[start_index + 1] == 601
