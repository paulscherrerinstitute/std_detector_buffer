import struct
from ctypes import (Structure, addressof, c_double, c_uint8, c_uint16,
                    c_uint32, c_uint64, memmove, sizeof)
from math import ceil

import numpy as np

EG_MAX_PAYLOAD = 4096
EG_GAPPIXELS_BETWEEN_CHIPS_X = 2
EG_GAPPIXELS_BETWEEN_CHIPS_Y = 2
EG_GAPPIXELS_BETWEEN_MODULES_X = 8
EG_GAPPIXELS_BETWEEN_MODULES_Y = 36
MODULE_X_SIZE = 256
MODULE_Y_SIZE = 512


class EgUdpPacket(Structure):
    _pack_ = 1
    _fields_ = [
        ("frame_num", c_uint64),
        ("exp_length", c_uint32),
        ("packet_number", c_uint32),
        ("detSpec1", c_uint64),
        ("timestamp", c_uint64),
        ("module_id", c_uint16),
        ("row", c_uint16),
        ("column", c_uint16),
        ("detSpec2", c_uint16),
        ("detSpec3", c_uint32),
        ("round_robin", c_uint16),
        ("detector_type", c_uint8),
        ("header_version", c_uint8)
    ]


class EGFrame(Structure):
    _pack_ = 1
    _fields_ = [("frame_index", c_uint64),
                ("n_missing_packets", c_uint64),
                ("bit_depth", c_uint16),
                ("row", c_uint16),
                ("column", c_uint16),
                ("exptime", c_uint32)]

    def __str__(self):
        return f"frame_index: {self.frame_index} " \
               f"bit_depth: {self.bit_depth} " \
               f"n_missing_packets: {self.n_missing_packets} " \
               f"exptime: {self.exptime}"


def eg_udp_packet_to_frame(packet, module_n_x_pixels, module_n_y_pixels, frame_n_packets, bit_depth):
    meta = EGFrame()

    meta.n_missing_packets = frame_n_packets
    meta.size_x = module_n_x_pixels
    meta.size_y = module_n_y_pixels

    meta.frame_index = packet.frame_num

    meta.frame_timestamp = packet.timestamp

    meta.bit_depth = bit_depth
    meta.row = packet.row
    meta.column = packet.column

    return meta


def calculate_udp_packet_info(image_width, image_height, bit_depth, n_modules):
    image_size_bytes = (image_width * image_height * bit_depth) / 8
    num_packets = image_size_bytes / EG_MAX_PAYLOAD

    pixel_size = bit_depth // 8  # Calculate pixel size in bytes
    pixels_per_packet = EG_MAX_PAYLOAD // pixel_size
    total_pixels = image_width * image_height
    total_packets = (total_pixels * bit_depth) // EG_MAX_PAYLOAD // 8

    image_size_bytes = (image_width * image_height * bit_depth) // 8

    num_packets = int(image_size_bytes / EG_MAX_PAYLOAD)
    if total_pixels % pixels_per_packet != 0:
        total_packets += 1

    last_packet_n_data_bytes = (total_pixels % pixels_per_packet) * pixel_size
    last_packet_starting_row = (total_packets - 1) * pixels_per_packet // image_width
    last_packet_n_rows = image_height % pixels_per_packet

    packet_info = {
        'packet_n_data_bytes': EG_MAX_PAYLOAD,
        'last_packet_starting_row': last_packet_starting_row,
        'frame_n_packets': num_packets,
        'packet_n_rows': pixels_per_packet // image_width,
        'last_packet_n_rows': last_packet_n_rows,
        'last_packet_n_data_bytes': last_packet_n_data_bytes,
        'total_img_size': image_size_bytes
    }
    return packet_info


class EigerConfigUdp:
    def __init__(self):
        self.id = 0
        self.name = f'EG05M-{self.id}'
        self.udp_port_base = 50020
        self.data_bytes_per_packet = 512 * 256 * 2
        self.slots = 10  # should be 1000 but for testing purposes 10 is enough
        self.buffer_size = self.data_bytes_per_packet * self.slots


class EigerConfigConverter:
    def __init__(self):
        udp = EigerConfigUdp()
        self.id = udp.id
        self.converter_index = 0
        self.name = 'EG05M-image'
        self.socket_name = 'EG05M-sync'
        self.data_bytes_per_packet = (4 * udp.data_bytes_per_packet) + (12 * 514) + (4 * 1024)
        self.udp_port_base = udp.udp_port_base
        self.slots = udp.slots
        self.buffer_size = self.data_bytes_per_packet * self.slots
