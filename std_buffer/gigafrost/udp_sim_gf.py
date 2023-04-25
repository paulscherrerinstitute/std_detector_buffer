import argparse
from socket import socket, AF_INET, SOCK_DGRAM
from time import time, sleep
import json

from std_buffer.gigafrost.data import GfUdpPacket, calculate_udp_packet_info

GF_N_MODULES = 8
PRINT_INTERVAL = 10


def init_gf_udp_packet(image_width):
    udp_packet = GfUdpPacket()

    udp_packet.frame_index = 0
    udp_packet.protocol_id = 203
    udp_packet.scan_id = 0
    udp_packet.scan_time = 100000
    udp_packet.sync_time = 200000
    udp_packet.image_timing = 300000

    udp_packet.quadrant_row_length_in_blocks = image_width // 2 // 12
    udp_packet.quadrant_rows = 0
    udp_packet.status_flags = 0
    udp_packet.image_status_flags = 0
    udp_packet.packet_starting_row = 0

    return udp_packet

def adjust_packet_for_module(udp_packet, i_module, image_height):
    quadrant_id = i_module // 2
    link_id = i_module % 2
    swap = 1 if quadrant_id % 2 == 0 else 0
    quadrant_height = image_height // 2
    # The correction to be applied to the imaging data (default = 5)
    # For more information: http://hpdi.gitpages.psi.ch/gf_docs/backend/quick_start.html
    corr_mode = 5

    udp_packet.status_flags = 0
    udp_packet.status_flags |= (quadrant_id << 6)
    udp_packet.status_flags |= (link_id << 5)
    udp_packet.status_flags |= (corr_mode << 2)

    udp_packet.status_flags |= quadrant_height >> 8

    # The last bit in the 'quadrant_row' is the 'swap' bit.
    udp_packet.quadrant_rows = (quadrant_height & 0xFF) + swap


def generate_data_for_packet(i_module, i_packet, n_rows_packet, n_cols_packet, n_packet_bytes):
    quadrant_id = i_module // 2
    link_id = i_module % 2

    packet_bytes = 0
    i_pixel = 0

    for i_module_row in range(n_rows_packet):
        for i_module_col in range(n_cols_packet):
            pixel_value = 0
            # Bit 11,10 == module_id
            pixel_value |= quadrant_id << 10
            # Bit 9 == link_id
            pixel_value |= link_id << 9
            # Bit 8 == i_packet % 2
            pixel_value |= (i_packet % 2) << 8
            # Bit 7,6,5,4 == module_row % 16
            pixel_value |= (i_module_row % 16) << 4
            # Bit 3,2,1,0 == module_col % 16
            pixel_value |= (i_module_col % 16)

            packet_bytes |= pixel_value << (i_pixel * 12)
            i_pixel += 1

    return packet_bytes.to_bytes(n_packet_bytes, 'little')


def generate_gf_udp_stream(output_address, start_udp_port, rep_rate=0.1,
                           image_height=2016, image_width=2016, n_images=None):
    time_to_sleep = 1 / rep_rate

    udp_socket = socket(AF_INET, SOCK_DGRAM)
    udp_packet = init_gf_udp_packet(image_width)

    udp_packet_info = calculate_udp_packet_info(image_height, image_width)
    n_packets = udp_packet_info['frame_n_packets']
    n_data_bytes = udp_packet_info['packet_n_data_bytes']
    n_data_bytes_last_packet = udp_packet_info['last_packet_n_data_bytes']
    n_rows_packet = udp_packet_info['packet_n_rows']
    n_rows_last_packet = udp_packet_info['last_packet_n_rows']
    n_cols_packet = image_width // 2


    if not n_images:
        n_images = float('inf')

    try:
        iteration_start = time()
        print_start = iteration_start

        while udp_packet.frame_index < n_images:

            # First n-1 packets
            for i_packet in range(n_packets-1):
                udp_packet.packet_starting_row = udp_packet_info['packet_n_rows'] * i_packet

                for i_module in range(GF_N_MODULES):
                    adjust_packet_for_module(udp_packet, i_module, image_height)
                    data = generate_data_for_packet(i_module, i_packet,
                                                    n_rows_packet=n_rows_packet,
                                                    n_cols_packet=n_cols_packet,
                                                    n_packet_bytes=n_data_bytes)

                    udp_socket.sendto(bytes(udp_packet)+data,
                                      (output_address, start_udp_port + i_module))

            # Last packet (may have different size and  number of lines)
            udp_packet.packet_starting_row = udp_packet_info['last_packet_starting_row']
            for i_module in range(GF_N_MODULES):
                adjust_packet_for_module(udp_packet, i_module, image_height)
                last_packet_data = generate_data_for_packet(i_module, n_packets-1,
                                                            n_rows_packet=n_rows_last_packet,
                                                            n_cols_packet=image_width//2,
                                                            n_packet_bytes=n_data_bytes_last_packet)

                udp_socket.sendto(bytes(udp_packet)+last_packet_data,
                                  (output_address, start_udp_port + i_module))

            iteration_end = time()
            iteration_time = iteration_end - iteration_start
            time_left_to_sleep = time_to_sleep - iteration_time

            # We do not sleep for less then 1ms.
            if time_left_to_sleep > 0.01:
                sleep(time_left_to_sleep)

            if iteration_end - print_start > PRINT_INTERVAL:
                print(f'Send all frames up to {udp_packet.frame_index}.')
                print_start = iteration_end

            udp_packet.frame_index += 1
            iteration_start = time()

    except KeyboardInterrupt:
        pass


def main():
    parser = argparse.ArgumentParser(description='Gigafrost udp stream generator.')
    parser.add_argument('detector_config_file', type=str, help='JSON file with detector configuration.')
    parser.add_argument('output_address', type=str, help='Address to send the UPD packets to.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)
    parser.add_argument('-n', '--n_images', type=int, default=None, help='Number of images to generate.')

    args = parser.parse_args()
    output_stream = args.output_address
    rep_rate = args.rep_rate
    n_images = args.n_images

    with open(args.detector_config_file, 'r') as input_file:
        config = json.load(input_file)

    start_udp_port = config['start_udp_port']
    image_height = config['image_pixel_height']
    image_width = config['image_pixel_width']

    n_images_str = 'unlimited' if n_images is None else n_images
    print(f'Starting simulated GF with rep_rate {rep_rate} on {output_stream} '
          f'with start_udp_port {start_udp_port} and image_shape {(image_height, image_width)} '
          f'for {n_images_str} images.')

    generate_gf_udp_stream(output_stream, start_udp_port, rep_rate, image_height, image_width, n_images)

    print('Stopping simulated stream.')


if __name__ == '__main__':
    main()
