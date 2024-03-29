from socket import socket, AF_INET, SOCK_DGRAM

import numpy as np
import pytest
import zmq.asyncio

from testing.fixtures import test_path, cleanup_jungfrau_shared_memory
from testing.communication import start_pull_communication
from testing.execution_helpers import build_command, run_command_in_parallel
from std_buffer.jungfrau.data import UdpPacket, JungfrauConfigUdp, JungfrauConfigConverter


def jungfrau_socket_address() -> tuple:
    return "127.0.0.1", JungfrauConfigUdp().udp_port_base + JungfrauConfigConverter().id


def get_converter_packet_array(output_buffer: memoryview, slot: int) -> np.ndarray:
    config_converter = JungfrauConfigConverter()
    config_converter.data_bytes_per_packet = 1024 * 512 * 4
    slot_start = slot * config_converter.data_bytes_per_packet
    data_of_slot = output_buffer[slot_start:slot_start + config_converter.data_bytes_per_packet]
    return np.ndarray((int(config_converter.data_bytes_per_packet / 4),), dtype='f4',
                      buffer=data_of_slot)


@pytest.mark.asyncio
async def test_udp_receiver_with_converter(test_path, cleanup_jungfrau_shared_memory):
    receiver_command = build_command('std_udp_recv_jf', test_path / 'jungfrau_detector_single_module.json', JungfrauConfigUdp().id)
    converter_command = build_command('std_data_convert_jf', test_path / 'jungfrau_detector_single_module.json',
                                      '-g', test_path / 'gains_1_pedestals_0.h5', JungfrauConfigUdp().id)

    ctx = zmq.asyncio.Context()
    packet = UdpPacket()

    with run_command_in_parallel(receiver_command), run_command_in_parallel(converter_command, 5):
        client_socket = socket(AF_INET, SOCK_DGRAM)
        with start_pull_communication(ctx, JungfrauConfigConverter()) as (output_buffer, sub_socket):
            for frame_id in range(10):  # send 10 frames
                packet.framenum = 2 + frame_id
                packet.bunchid = 5.0 + frame_id
                msg = sub_socket.recv()  # future for the asynchronous message receive

                # send via UDP 128 packets to std_udp_recv_jf
                for i in range(JungfrauConfigUdp().packets_per_frame):
                    packet.packetnum = i
                    for j in range(len(packet.data)):
                        packet.data[j] = i
                    client_socket.sendto(packet, jungfrau_socket_address())

                # await trigger from std_data_convert after data has been successfully converted
                received_id = await msg
                assert np.frombuffer(received_id, dtype='i8')[0] == packet.bunchid

                converted_data = get_converter_packet_array(output_buffer, int(packet.bunchid))
                for i in range(JungfrauConfigUdp().packets_per_frame):
                    for j in range(len(packet.data)):
                        assert converted_data[i * len(packet.data) + j] == i
