import json
from collections import defaultdict
from math import ceil
from pathlib import Path
import struct
from std_buffer.gigafrost.data import GFFrame, GF_MAX_PAYLOAD, GfUdpPacket



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
    # packet_n_rows = int(min(GF_MAX_PAYLOAD / 1.5 / module_n_x_pixel, module_n_y_pixel))

    # Do NOT optimize these expressions. The exact form of this calculation is important due to rounding.
    n_12pixel_blocks = module_n_x_pixel // 12
    n_cache_line_blocks = int((GF_MAX_PAYLOAD / (36 * n_12pixel_blocks)) * n_12pixel_blocks / 2)
    # Each cachel line block (64 bytes) has 48 pixels (12 bit pixels)
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


def calculate_frame_info_original(image_width, image_height, link_id=0):
    # Each line of final image is composed by 2 quadrants side by side.
    module_size_x = image_width // 2
    # Each quadrant is composed by 2 modules streaming interleaved image lines.
    module_size_y = image_height // 4

    count_x_12ppq = module_size_x / 12
    udp_blocks_48p = int((GF_MAX_PAYLOAD / (36 * count_x_12ppq)) * count_x_12ppq / 2)
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

    return int(payload), int(last_packet_offset), -1, int(nrows), -1


if __name__ == '__main__':
    for y in range(4, 2016, 4):
        for x in range(48, 2016, 12):
            new_info = calculate_frame_info(x, y)
            old_info = calculate_frame_info_original(x, y)

            n_bytes_old, n_bytes_new =old_info[0], new_info['packet_n_data_bytes']
            starting_old, starting_new = old_info[1]/2, new_info['last_packet_starting_row']
            n_row_old, n_row_new = old_info[3], new_info['packet_n_rows']

            if n_bytes_new != n_bytes_old or starting_old != starting_new or n_row_old != n_row_new:
                print(x, y, 'bytes', n_bytes_old, n_bytes_new, 'start', starting_old,starting_new, 'n_rows', n_row_old, n_row_new)

    image_pixel_width = 672
    image_pixel_height = 128

    bin_files = Path(__file__).parent.absolute() / 'udp_dumps' / f'{image_pixel_width}_{image_pixel_height}'
    files = [
        open(f'{bin_files}/2000.dat', 'rb'),
        open(f'{bin_files}/2001.dat', 'rb'),
        # open(f'{bin_files}/2002.dat', 'rb'),
        # open(f'{bin_files}/2003.dat', 'rb'),
        # open(f'{bin_files}/2004.dat', 'rb'),
        # open(f'{bin_files}/2005.dat', 'rb'),
        # open(f'{bin_files}/2006.dat', 'rb'),
        # open(f'{bin_files}/2007.dat', 'rb'),
    ]

    module_n_x_pixel = image_pixel_width // 2
    module_n_y_pixel = image_pixel_height // 2 // 2

    frame_info = calculate_frame_info(image_pixel_width, image_pixel_height)
    print(frame_info)
    frame_info_original = calculate_frame_info_original(image_pixel_width, image_pixel_height)

    # Check if both frame_info routines give you the same numbers.
    # for i in range(len(frame_info)):
    #     print(frame_info[i], frame_info_original[i])
    #     # assert frame_info[i] == frame_info_original[i]

    cache = defaultdict(dict)

    for i in range(frame_info['frame_n_packets']*2):

        for input_file in files:
            # uint64_t with the number of bytes per packet.
            n_bytes_to_read = struct.unpack('Q', input_file.read(8))[0]
            # assert(n_bytes_to_read == frame_info['packet_n_data_bytes'] + 32)
            n_data_bytes = n_bytes_to_read - 32

            data = input_file.read(n_bytes_to_read)
            packet = GfUdpPacket.from_buffer_copy(data)
            frame = gf_udp_packet_to_frame(packet, module_n_x_pixel, module_n_y_pixel, frame_info['frame_n_packets'])
            key = (frame.frame_index, frame.quadrant_id, frame.link_id, input_file.name)

            # # Print frame offset data points
            # frame_offset = packet.packet_starting_row * module_n_x_pixel * 1.5
            # print('starting_row', packet.packet_starting_row,
            #       'frame_offset', frame_offset,
            #       'i_packet', frame_offset/frame_info['packet_n_data_bytes'])

            if key not in cache:
                cache[key] = defaultdict(dict)

            cache[key][n_data_bytes].update({
                'packet_starting_row': packet.packet_starting_row
            })
            # print(n_data_bytes, packet.packet_starting_row)

            if 'n_packets' not in cache[key][n_data_bytes]:
                cache[key][n_data_bytes]['n_packets'] = 0
            cache[key][n_data_bytes]['n_packets'] += 1

    print('key: (frame_index, quadrant_id, link_id)')
    for key, value in cache.items():
        print(key[:-1], json.dumps(value))

    for file in files:
        file.close()
