from ctypes import Structure, c_double, c_uint32, c_uint64, c_uint16, c_uint8, memmove, addressof, sizeof
from math import ceil
import struct 
import numpy as np

EG_MAX_PAYLOAD = 4096
EG_GAPPIXELS_BETWEEN_CHIPS_X = 2
EG_GAPPIXELS_BETWEEN_CHIPS_Y = 2
EG_GAPPIXELS_BETWEEN_MODULES_X = 8
EG_GAPPIXELS_BETWEEN_MODULES_Y = 36
MODULE_X_SIZE = 256
MODULE_Y_SIZE = 512

class EgUdpPacket(Structure):
    _comparable_fields = ['frame_num', 'exp_length', 'packet_number',
                          'timestamp', 'module_id',
                          'row', 'column',
                          'detector_type', 'header_version']

    _pack_ = 1
    _fields_ = [("frame_num", c_uint64),
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
                ("header_version", c_uint8)]

    def unpack(self, data):
        memmove(addressof(self), data, sizeof(self))

    def pack(self):
        return struct.pack('!QIIBBHHHBIBB', self.frame_num, self.exp_length,
                           self.packet_number, self.detSpec1, self.timestamp,
                           self.module_id, self.row, self.column, self.detSpec2,
                           self.detSpec3, self.round_robin, self.detector_type,
                           self.header_version)

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
                ("row", c_uint16),
                ("column", c_uint16),
                ("exptime", c_uint32)]

    def __str__(self):
        return f"frame_index: {self.frame_index} "\
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

# def calculate_udp_packet_info(n_modules, image_pixel_height, image_pixel_width, bit_depth):
#     # Calculations for a 0.5M Eiger
#     # Each line of final image is composed by 4 chips (256x256) side by side and gap pixels.
#     # Each column of the final image is composed by 2 chips (256x256) on top of each other and gap pixels.
#     module_n_x_pixel = MODULE_X_SIZE
#     module_n_y_pixel = MODULE_Y_SIZE

#     # Max udp packet payload divided by the module row size in bytes or module Y size if smaller.
#     packet_n_rows = int(min(EG_MAX_PAYLOAD * module_n_x_pixel / bit_depth / 8 , module_n_y_pixel))

#     # Calculate the number of data bytes per packet.
#     packet_n_data_bytes = int(module_n_x_pixel * packet_n_rows * bit_depth / 8)

#     # Number of rows in a frame divided by the number of rows in a packet.
#     frame_n_packets = ceil(module_n_y_pixel / packet_n_rows)

#     # Calculate if the last packet has the same number of rows as the rest of the packets.
#     last_packet_n_rows = module_n_y_pixel % packet_n_rows or packet_n_rows

#     last_packet_n_data_bytes = int(module_n_x_pixel * last_packet_n_rows * bit_depth / 8)

#     # Get offset of last packet in frame to know when to commit frame.
#     last_packet_starting_row = module_n_y_pixel - last_packet_n_rows

#     return {'packet_n_data_bytes': packet_n_data_bytes,
#             'last_packet_starting_row': last_packet_starting_row,
#             'frame_n_packets': frame_n_packets,
#             'packet_n_rows': packet_n_rows,
#             'last_packet_n_rows': last_packet_n_rows,
#             'last_packet_n_data_bytes': last_packet_n_data_bytes}

def calculate_udp_packet_info(bit_depth):
    
    pixel_size = bit_depth // 8  # Calculate pixel size in bytes

    pixels_per_packet = EG_MAX_PAYLOAD // pixel_size
    total_pixels = MODULE_X_SIZE * MODULE_Y_SIZE
    total_packets = total_pixels // pixels_per_packet
    total_img_size = total_pixels * bit_depth
    if total_pixels % pixels_per_packet != 0:
        total_packets += 1

    last_packet_n_data_bytes = (total_pixels % pixels_per_packet) * pixel_size
    last_packet_starting_row = (total_packets - 1) * pixels_per_packet // MODULE_X_SIZE
    last_packet_n_rows = MODULE_Y_SIZE % pixels_per_packet
    # print(total_packets, pixels_per_packet)
    # frame_n_packets = total_packets // (MODULE_X_SIZE // pixels_per_packet)
    # frame_n_packets = total_packets // (max(1, MODULE_X_SIZE // pixels_per_packet))
    frame_n_packets = ceil(MODULE_Y_SIZE / (pixels_per_packet // MODULE_X_SIZE))


    packet_info = {
        'packet_n_data_bytes': EG_MAX_PAYLOAD,
        'last_packet_starting_row': last_packet_starting_row,
        'frame_n_packets': frame_n_packets,
        'packet_n_rows': pixels_per_packet // MODULE_X_SIZE,
        'last_packet_n_rows': last_packet_n_rows,
        'last_packet_n_data_bytes': last_packet_n_data_bytes,
        'total_img_size':total_img_size
    }

    return packet_info


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
