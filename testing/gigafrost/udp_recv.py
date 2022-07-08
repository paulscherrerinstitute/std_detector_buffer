import json
from collections import defaultdict
from ctypes import Structure, c_uint8, c_uint32, c_uint16, c_uint64
from math import ceil
from pathlib import Path

GF_MAX_PAYLOAD = 7400


class GfUdpPacket(Structure):
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
        return f"frame_index: {self.frame_index}; " \
               f"quadrant_rows: {(self.quadrant_rows >>1) << 1}; " \
               f"quadrant_row_length_in_blocks: {self.quadrant_row_length_in_blocks}; " \
               f"packet_starting_row: {self.packet_starting_row}; "


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


def calculate_frame_info(image_pixel_width, image_pixel_height):
    # Each line of final image is composed by 2 quadrants side by side.
    module_n_x_pixel = image_pixel_width // 2

    # Each quadrant is composed by 2 modules streaming interleaved image lines.
    module_n_y_pixel = image_pixel_height // 2 // 2

    # Maximum udp packet payload divided by the module row size in bytes or module Y size if smaller.
    packet_n_rows = int(min(GF_MAX_PAYLOAD / 1.5 / module_n_x_pixel, module_n_y_pixel))

    # Calculate the number of data bytes per packet.
    packet_n_data_bytes = int(module_n_x_pixel * packet_n_rows * 1.5)
    if packet_n_rows % 2 == 1 and module_n_x_pixel % 48 != 0:
        # Add bytes for 24 pixels (24 * 1.5) padding.
        packet_n_data_bytes += 36

    # Number of rows in a frame divided by the number of rows in a packet.
    frame_n_packets = ceil(module_n_y_pixel / packet_n_rows)

    # Calculate if the last packet has the same number of rows as the rest of the packets.
    last_packet_n_rows = module_n_y_pixel % packet_n_rows or packet_n_rows

    # Get offset of last packet in frame to know when to commit frame.
    last_packet_starting_row = module_n_y_pixel - last_packet_n_rows

    return packet_n_data_bytes, last_packet_starting_row, frame_n_packets


def calculate_frame_info_original(image_width, image_height, link_id=0):
    # Each line of final image is composed by 2 quadrants side by side.
    module_size_x = image_width // 2
    # Each quadrant is composed by 2 modules streaming interleaved image lines.
    module_size_y = image_height // 4

    count_x_12ppq = module_size_x / 12
    udp_blocks_48p = (GF_MAX_PAYLOAD / (36 * count_x_12ppq)) * count_x_12ppq / 2
    nrows = int(min(udp_blocks_48p * 48 / module_size_x, module_size_y))

    if nrows % 2 == 0:
        payload = module_size_x * nrows * 1.5
    else:
        if module_size_x % 48 == 0:
            payload = module_size_x * nrows * 1.5
        else:
            payload = (module_size_x * nrows * 1.5) + (24 * 1.5)

    stride = 2
    if nrows == module_size_y:
        last_packet_offset = 0 * stride + link_id
    elif module_size_y % nrows == 0:
        last_packet_offset = (module_size_y - nrows) * stride + link_id
    else:
        last_packet_offset = (module_size_y - module_size_y % nrows) * stride + link_id

    return int(payload), int(last_packet_offset), int(nrows)


if __name__ == '__main__':
    bin_files = Path(__file__).parent.absolute() / 'udp_dumps'
    files = [
        open(f'{bin_files}/10.0.0.100_2000.dat', 'rb'),
        open(f'{bin_files}/10.0.0.100_2001.dat', 'rb'),
        open(f'{bin_files}/10.0.0.100_2002.dat', 'rb'),
        open(f'{bin_files}/10.0.0.100_2003.dat', 'rb'),
        open(f'{bin_files}/10.4.0.100_2004.dat', 'rb'),
        open(f'{bin_files}/10.4.0.100_2005.dat', 'rb'),
        open(f'{bin_files}/10.4.0.100_2006.dat', 'rb'),
        open(f'{bin_files}/10.4.0.100_2007.dat', 'rb'),
    ]

    image_pixel_height = 2016
    image_pixel_width = 2016
    module_n_x_pixel = image_pixel_width // 2
    module_n_y_pixel = image_pixel_height // 2 // 2

    frame_info = calculate_frame_info(image_pixel_height, image_pixel_width)
    frame_info_original = calculate_frame_info_original(image_pixel_height, image_pixel_width)

    # Check if both frame_info routines give you the same numbers.
    for i in range(len(frame_info)):
        print(frame_info[i], frame_info_original[i])
        # assert frame_info[i] == frame_info_original[i]

    packet_n_data_bytes, last_packet_starting_row, frame_n_packets = frame_info
    # 32 bytes UDP header.
    packet_n_bytes = packet_n_data_bytes + 32
    print(
        'packet_n_data_bytes', packet_n_data_bytes,
        'last_packet_starting_row', last_packet_starting_row,
        'frame_n_packets', frame_n_packets,
        'packet_n_bytes', packet_n_bytes
    )

    cache = defaultdict(dict)

    for i in range(frame_n_packets*2):

        for input_file in files:

            data = input_file.read(packet_n_bytes)
            packet = GfUdpPacket.from_buffer_copy(data)
            frame = gf_udp_packet_to_frame(packet, module_n_x_pixel, module_n_y_pixel, frame_n_packets)

            key = (frame.frame_index, frame.quadrant_id, frame.link_id, input_file.name)
            cache[key].update({
                'quadrant_id': frame.quadrant_id,
                'quadrant_row_length_in_blocks': packet.quadrant_row_length_in_blocks,
                'last_packet_starting_row': packet.packet_starting_row
            })

            if 'n_packets' not in cache[key]:
                cache[key]['n_packets'] = 0
            cache[key]['n_packets'] += 1

    print('key: (frame_index, quadrant_id, link_id)')
    for key, value in cache.items():
        print(key[:-1], json.dumps(value))

    for file in files:
        file.close()
