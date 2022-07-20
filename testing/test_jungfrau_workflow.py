import time

import pytest

from testing.execution_helpers import build_command, run_command_in_parallel
from testing.fixtures import test_path, cleanup_jungfrau_shared_memory


class JungfrauConfigUdp:
    id = 1
    name = f'jungfrau{id}'
    udp_port_base = 50020
    meta_bytes_per_packet = 48
    data_bytes_per_packet = 8192 * 128
    bytes_per_packet = meta_bytes_per_packet + data_bytes_per_packet
    packets_per_frame = 128
    slots = 10  # should be 1000 but for testing purposes 10 is enough
    buffer_size = bytes_per_packet * slots


class JungfrauConfigConverter:
    id = JungfrauConfigUdp.id
    name = f'jungfrau{id}-converted'
    data_bytes_per_packet = JungfrauConfigUdp.data_bytes_per_packet * 2
    udp_port_base = JungfrauConfigUdp.udp_port_base
    slots = JungfrauConfigUdp.slots
    meta_bytes_per_packet = JungfrauConfigUdp.meta_bytes_per_packet
    bytes_per_packet = meta_bytes_per_packet + data_bytes_per_packet
    buffer_size = bytes_per_packet * slots


@pytest.mark.asyncio
async def test_udp_receiver_with_converter(test_path, cleanup_jungfrau_shared_memory):
    command = build_command('std_udp_recv_jf', test_path / 'jungfrau_detector.json', JungfrauConfigUdp.id)

    with run_command_in_parallel(command):
        time.sleep(1)
