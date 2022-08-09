import json
from socket import socket, AF_INET, SOCK_DGRAM
from time import sleep

import pytest as pytest
import numpy as np

from testing.execution_helpers import build_command, run_command_in_parallel
from testing.jungfrau.data import JungfrauConfigUdp, UdpPacket
from tools.std_stream_recv_binary import StdStreamRecvBinary
from testing.fixtures import test_path


@pytest.mark.asyncio
async def test_std_stream_send_binary_with_recv(test_path):
    det_config_filename = test_path / 'jungfrau_detector.json'
    with open(det_config_filename, 'r') as input_file:
        det_config = json.load(input_file)

    image_height = det_config['image_pixel_height']
    image_width = det_config['image_pixel_width']

    # TODO: This is hardcoded in the sender at the moment.
    module_id = 0
    pulse_id = 10
    frame_num = 100

    recv_cmd = build_command('std_udp_recv_jf', det_config_filename, module_id)
    converter_command = build_command('std_data_convert', test_path / 'jungfrau_detector.json',
                                      test_path / 'gains_1_pedestals_0.h5', module_id)
    stream_cmd = build_command('std_stream_send_binary', det_config_filename, 'ipc://send_binary_test')

    udp_packet = UdpPacket()
    udp_packet.moduleID = module_id

    with run_command_in_parallel(recv_cmd), \
         run_command_in_parallel(converter_command), \
         run_command_in_parallel(stream_cmd):
        with StdStreamRecvBinary('ipc://send_binary_test') as input_stream:
            with socket(AF_INET, SOCK_DGRAM) as udp_socket:
                # send via UDP 128 packets to std_udp_recv_jf
                for i_packet in range(JungfrauConfigUdp.packets_per_frame):
                    udp_packet.packetnum = i_packet
                    udp_packet.framnum = frame_num
                    udp_packet.bunchid = pulse_id
                    for i_data in range(len(udp_packet.data)):
                        udp_packet.data[i_data] = i_packet

                    udp_socket.sendto(udp_packet, ('127.0.0.1', 50020))

            meta, data = input_stream.recv()

            assert(meta.id == pulse_id)
            assert(meta.height == image_height)
            assert(meta.width == image_width)
            # TODO: Currently this is hardcoded, change type dynamically.
            assert(meta.get_dtype_description() == 'float32')
            assert(meta.get_status_description() == 'good_image')

            n_data_points_per_packet = len(udp_packet.data)
            for i_packet in range(JungfrauConfigUdp.packets_per_frame):
                for i_data in range(n_data_points_per_packet):
                    offset = (i_packet * n_data_points_per_packet) + i_data
                    row_offset = offset // image_width
                    column_offset = offset % image_width

                    np.testing.assert_almost_equal(data[row_offset][column_offset], i_packet)
