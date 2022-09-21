import json
from collections import defaultdict
from pathlib import Path
import struct
from std_buffer.gigafrost.data import GF_MAX_PAYLOAD, GfUdpPacket, gf_udp_packet_to_frame, calculate_udp_packet_info


def calculate_udp_packet_info_original(image_height, image_width, link_id=0):
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
            new_info = calculate_udp_packet_info(y, x)
            old_info = calculate_udp_packet_info_original(y, x)

            n_bytes_old, n_bytes_new =old_info[0], new_info['packet_n_data_bytes']
            starting_old, starting_new = old_info[1]/2, new_info['last_packet_starting_row']
            n_row_old, n_row_new = old_info[3], new_info['packet_n_rows']

            if n_bytes_new != n_bytes_old or starting_old != starting_new or n_row_old != n_row_new:
                print(x, y, 'bytes', n_bytes_old, n_bytes_new, 'start', starting_old,starting_new, 'n_rows', n_row_old, n_row_new)

    image_pixel_width = 2016
    image_pixel_height = 2016
    n_images = 2

    bin_files = Path(__file__).parent.absolute() / 'udp_dumps' / f'{image_pixel_width}_{image_pixel_height}'
    files = [
        open(f'{bin_files}/2000.dat', 'rb'),
        open(f'{bin_files}/2001.dat', 'rb'),
        open(f'{bin_files}/2002.dat', 'rb'),
        open(f'{bin_files}/2003.dat', 'rb'),
        open(f'{bin_files}/2004.dat', 'rb'),
        open(f'{bin_files}/2005.dat', 'rb'),
        open(f'{bin_files}/2006.dat', 'rb'),
        open(f'{bin_files}/2007.dat', 'rb'),
    ]

    module_n_x_pixel = image_pixel_width // 2
    module_n_y_pixel = image_pixel_height // 2 // 2

    frame_info = calculate_udp_packet_info(image_pixel_height, image_pixel_width)
    print(frame_info)
    frame_info_original = calculate_udp_packet_info_original(image_pixel_height, image_pixel_width)

    # # Check if both frame_info routines give you the same numbers.
    # for i in range(len(frame_info)):
    #     print(frame_info[i], frame_info_original[i])
    #     assert frame_info[i] == frame_info_original[i]

    cache = defaultdict(dict)

    for i in range(frame_info['frame_n_packets'] * n_images):

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
                'packet_starting_row': packet.packet_starting_row,
                'swap': frame.swapped_rows,
                'status_flags': packet.status_flags
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
