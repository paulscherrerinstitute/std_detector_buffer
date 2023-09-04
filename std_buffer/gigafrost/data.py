from ctypes import Structure, c_uint32, c_uint64, c_uint16, c_uint8
from math import ceil

import numpy as np

GF_MAX_PAYLOAD = 7400


class GfUdpPacket(Structure):
    _comparable_fields = ['protocol_id', 'quadrant_row_length_in_blocks', 'quadrant_rows',
                          'status_flags', 'frame_index', 'image_status_flags',
                          'packet_starting_row']

    _pack_ = 1
    _fields_ = [("protocol_id", c_uint8),
                ("quadrant_row_length_in_blocks", c_uint8),
                ("quadrant_rows", c_uint8),
                ("status_flags", c_uint8),
                ("scan_id", c_uint32),
                ("frame_index", c_uint32),
                ("image_status_flags", c_uint16),
                ("packet_starting_row", c_uint16),
                ("image_timing", c_uint64),
                ("sync_time", c_uint32),
                ("scan_time", c_uint32)]

    def __str__(self):
        output_string = ''
        for field_name in self._comparable_fields:
            output_string += f'{field_name}: {getattr(self, field_name)}; '
        return output_string

    def __eq__(self, other):
        for field_name in self._comparable_fields:
            if getattr(self, field_name) != getattr(other, field_name):
                return False
        return True


class GFFrame(Structure):
    _pack_ = 1
    _fields_ = [("frame_index", c_uint64),
                ("n_missing_packets", c_uint64),
                ("scan_id", c_uint32),
                ("size_x", c_uint32),
                ("size_y", c_uint32),
                ("scan_time", c_uint32),
                ("sync_time", c_uint32),
                ("frame_timestamp", c_uint64),
                ("exposure_time", c_uint64),
                ("swapped_rows", c_uint32),
                ("link_id", c_uint32),
                ("corr_mode", c_uint32),
                ("quadrant_id", c_uint32),
                ("rpf", c_uint32),
                ("do_not_store", c_uint32)]

    def __str__(self):
        return f"link_id: {self.link_id} " \
               f"quadrant_id: {self.quadrant_id}"


def gf_udp_packet_to_frame(packet, module_n_x_pixels, module_n_y_pixels, frame_n_packets):
    meta = GFFrame()

    meta.n_missing_packets = frame_n_packets
    meta.size_x = module_n_x_pixels
    meta.size_y = module_n_y_pixels

    meta.frame_index = packet.frame_index
    meta.scan_id = packet.scan_id
    meta.scan_time = packet.scan_time
    meta.sync_time = packet.sync_time

    meta.frame_timestamp = (packet.image_timing & 0x000000FFFFFFFFFF)
    meta.exposure_time = (packet.image_timing & 0xFFFFFF0000000000) >> 40

    meta.swapped_rows = packet.quadrant_rows & 0b1
    meta.quadrant_id = (packet.status_flags & 0b11000000) >> 6
    meta.link_id = (packet.status_flags & 0b00100000) >> 5
    meta.corr_mode = (packet.status_flags & 0b00011100) >> 2

    meta.do_not_store = packet.image_status_flags & 0x8000 >> 15

    return meta


def calculate_udp_packet_info(image_pixel_height, image_pixel_width):
    # Each line of final image is composed by 2 quadrants side by side.
    module_n_x_pixel = image_pixel_width // 2

    # Each quadrant is composed by 2 modules streaming interleaved image lines.
    module_n_y_pixel = image_pixel_height // 2 // 2

    # Max udp packet payload divided by the module row size in bytes or module Y size if smaller.
    # packet_n_rows = int(min(GF_MAX_PAYLOAD / 1.5 / module_n_x_pixel, module_n_y_pixel))

    # Do NOT optimize these expressions due to rounding.
    n_12pixel_blocks = module_n_x_pixel // 12
    n_cache_line_blocks = int((GF_MAX_PAYLOAD / (36 * n_12pixel_blocks)) * n_12pixel_blocks / 2)
    # Each cache line block (64 bytes) has 48 pixels (12 bit pixels)
    packet_n_rows = int(min(n_cache_line_blocks * 48 / module_n_x_pixel, module_n_y_pixel))

    # Calculate the number of data bytes per packet.
    packet_n_data_bytes = int(module_n_x_pixel * packet_n_rows * 1.5)
    if packet_n_rows % 2 == 1 and module_n_x_pixel % 48 != 0:
        # Add bytes for 24 pixels (24 * 1.5) padding.
        packet_n_data_bytes += 36

    # Number of rows in a frame divided by the number of rows in a packet.
    frame_n_packets = ceil(module_n_y_pixel / packet_n_rows)

    # Calculate if the last packet has the same number of rows as the rest of the packets.
    last_packet_n_rows = module_n_y_pixel % packet_n_rows or packet_n_rows

    last_packet_n_data_bytes = int(module_n_x_pixel * last_packet_n_rows * 1.5)
    if last_packet_n_rows % 2 == 1 and module_n_x_pixel % 48 != 0:
        # Add bytes for 24 pixels (24 * 1.5) padding.
        last_packet_n_data_bytes += 36

    # Get offset of last packet in frame to know when to commit frame.
    last_packet_starting_row = module_n_y_pixel - last_packet_n_rows

    return {'packet_n_data_bytes': packet_n_data_bytes,
            'last_packet_starting_row': last_packet_starting_row,
            'frame_n_packets': frame_n_packets,
            'packet_n_rows': packet_n_rows,
            'last_packet_n_rows': last_packet_n_rows,
            'last_packet_n_data_bytes': last_packet_n_data_bytes}


class GigafrostConfigUdp:
    def __init__(self):
        self.quadrant_id = 0
        self.id = 0
        self.name = f'GF2-{self.id}'
        self.udp_port_base = 50020
        self.image_pixel_height = 2016
        self.image_pixel_width = 2016
        self.meta_bytes_per_packet = 64
        self.data_bytes_per_packet = calculate_udp_packet_info(self.image_pixel_height, self.image_pixel_width)[
                                         'packet_n_data_bytes'] * \
                                     (calculate_udp_packet_info(self.image_pixel_height, self.image_pixel_width)[
                                          'frame_n_packets'] - 1) + \
                                     calculate_udp_packet_info(self.image_pixel_height, self.image_pixel_width)[
                                         'last_packet_n_data_bytes']
        self.slots = 10  # should be 1000 but for testing purposes 10 is enough
        self.buffer_size = self.data_bytes_per_packet * self.slots


class GigafrostConfigConverter:
    def __init__(self):
        udp = GigafrostConfigUdp()
        self.id = udp.id
        self.name = 'GF2-image'
        self.socket_name = 'GF2-sync'
        self.udp_port_base = udp.udp_port_base
        self.slots = udp.slots
        self.data_bytes_per_packet = udp.image_pixel_width * udp.image_pixel_height * 2
        self.buffer_size = self.data_bytes_per_packet * self.slots
