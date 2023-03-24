from ctypes import Structure, c_double, c_uint32, c_uint64, c_uint16, c_uint8
from math import ceil

import numpy as np

EG_MAX_PAYLOAD = 4096
EG_GAPPIXELS_BETWEEN_CHIPS_X = 2
EG_GAPPIXELS_BETWEEN_CHIPS_Y = 2
EG_GAPPIXELS_BETWEEN_MODULES_X = 8
EG_GAPPIXELS_BETWEEN_MODULES_Y = 36

class EgUdpPacket(Structure):
    _comparable_fields = ['framenum', 'exptime', 'packetnum',
                          'bunchid', 'timestamp', 'moduleID',
                          'row', 'column', 'reserved', 'roundRobin',
                          'detectortype', 'headerVersion']

    _pack_ = 1
    _fields_ = [("framenum", c_uint64),
                ("exptime", c_uint32),
                ("packetnum", c_uint32),
                ("bunchid", c_double),
                ("timestamp", c_uint64),
                ("moduleID", c_uint16),
                ("row", c_uint16),
                ("column", c_uint16),
                ("reserved", c_uint16),
                ("debug", c_uint32),
                ("roundRobin", c_uint16),
                ("detectortype", c_uint8),
                ("headerVersion", c_uint8)]

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


class EGFrame(Structure):
    _pack_ = 1
    _fields_ = [("frame_index", c_uint64),
                ("n_missing_packets", c_uint64),
                ("bit_depth", c_uint16),
                ("pos_y", c_uint16),
                ("pos_x", c_uint16),
                ("exptime", c_uint32),
                ("bunchid", c_double),
                ("debug", c_uint32)]

    def __str__(self):
        return f"bunchid: {self.bunchid} " \
               f"bit_depth: {self.bit_depth} " \
               f"exptime: {self.exptime}"


def eg_udp_packet_to_frame(packet, module_n_x_pixels, module_n_y_pixels, frame_n_packets):
    meta = EGFrame()

    meta.n_missing_packets = frame_n_packets
    meta.size_x = module_n_x_pixels
    meta.size_y = module_n_y_pixels

    meta.frame_index = packet.framenun

    meta.frame_timestamp = packet.timestamp
    
    meta.bit_depth = packet.bit_depth
    meta.pos_y = packet.row
    meta.pos_x = packet.column
    
    meta.exposure_time = packet.exptime
    meta.bunchid = packet.bunchid
    meta.debug = packet.debug
    

    return meta

# //TODO -> CALCULATE UDP PACKET INFO FOR EIGER
# def calculate_udp_packet_info(image_pixel_height, image_pixel_width):
#     # Each line of final image is composed by 2 quadrants side by side.
#     module_n_x_pixel = image_pixel_width // 2

#     # Each quadrant is composed by 2 modules streaming interleaved image lines.
#     module_n_y_pixel = image_pixel_height // 2 // 2

#     # Max udp packet payload divided by the module row size in bytes or module Y size if smaller.
#     # packet_n_rows = int(min(GF_MAX_PAYLOAD / 1.5 / module_n_x_pixel, module_n_y_pixel))

#     # Do NOT optimize these expressions due to rounding.
#     n_12pixel_blocks = module_n_x_pixel // 12
#     n_cache_line_blocks = int((EG_MAX_PAYLOAD / (36 * n_12pixel_blocks)) * n_12pixel_blocks / 2)
#     # Each cache line block (64 bytes) has 48 pixels (12 bit pixels)
#     packet_n_rows = int(min(n_cache_line_blocks * 48 / module_n_x_pixel, module_n_y_pixel))

#     # Calculate the number of data bytes per packet.
#     packet_n_data_bytes = int(module_n_x_pixel * packet_n_rows * 1.5)
#     if packet_n_rows % 2 == 1 and module_n_x_pixel % 48 != 0:
#         # Add bytes for 24 pixels (24 * 1.5) padding.
#         packet_n_data_bytes += 36

#     # Number of rows in a frame divided by the number of rows in a packet.
#     frame_n_packets = ceil(module_n_y_pixel / packet_n_rows)

#     # Calculate if the last packet has the same number of rows as the rest of the packets.
#     last_packet_n_rows = module_n_y_pixel % packet_n_rows or packet_n_rows

#     last_packet_n_data_bytes = int(module_n_x_pixel * last_packet_n_rows * 1.5)
#     if last_packet_n_rows % 2 == 1 and module_n_x_pixel % 48 != 0:
#         # Add bytes for 24 pixels (24 * 1.5) padding.
#         last_packet_n_data_bytes += 36

#     # Get offset of last packet in frame to know when to commit frame.
#     last_packet_starting_row = module_n_y_pixel - last_packet_n_rows

#     return {'packet_n_data_bytes': packet_n_data_bytes,
#             'last_packet_starting_row': last_packet_starting_row,
#             'frame_n_packets': frame_n_packets,
#             'packet_n_rows': packet_n_rows,
#             'last_packet_n_rows': last_packet_n_rows,
#             'last_packet_n_data_bytes': last_packet_n_data_bytes}


class EigerConfigUdp:
    id = 0
    name = f'EG05M-{id}'
    udp_port_base = 50020
    data_bytes_per_packet = 512 * 256 * 2
    # packets_per_frame = 128
    slots = 10  # should be 1000 but for testing purposes 10 is enough
    buffer_size = data_bytes_per_packet * slots

class EigerConfigConverter:
    id = EigerConfigUdp.id
    converter_index = 0
    name = 'EG05M-image'
    socket_name = 'EG05M-sync'
    data_bytes_per_packet = (4 * EigerConfigUdp.data_bytes_per_packet) + (12 * 514) + (4 * 1024)
    udp_port_base = EigerConfigUdp.udp_port_base
    slots = EigerConfigUdp.slots
    buffer_size = data_bytes_per_packet * slots

def get_converter_buffer_data(buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * EigerConfigConverter.data_bytes_per_packet
    data_of_slot = buffer[slot_start:slot_start + EigerConfigConverter.data_bytes_per_packet]
    return np.ndarray((int(EigerConfigConverter.data_bytes_per_packet / 2),), dtype='u2',
                      buffer=data_of_slot)


def get_udp_packet_array(input_buffer: memoryview, slot: int) -> np.ndarray:
    slot_start = slot * EigerConfigUdp.data_bytes_per_packet
    data_of_slot = input_buffer[slot_start:slot_start + EigerConfigUdp.data_bytes_per_packet]
    return np.ndarray((int(EigerConfigUdp.data_bytes_per_packet),), dtype='i1', buffer=data_of_slot)
