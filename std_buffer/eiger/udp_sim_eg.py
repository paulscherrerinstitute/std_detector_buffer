import argparse
from functools import cache
from socket import socket, AF_INET, SOCK_DGRAM
from time import time, sleep
import json
import ctypes



from std_buffer.eiger.data import EgUdpPacket, calculate_udp_packet_info

PRINT_INTERVAL = 10

def init_eg_udp_packet(module_id):
    udp_packet = EgUdpPacket()

    udp_packet.frame_num = 0
    udp_packet.exp_length = 0
    udp_packet.packet_number = 0
    udp_packet.detSpec1 = 0
    udp_packet.timestamp = 100000
    udp_packet.module_id = 0
    udp_packet.row = 0
    udp_packet.column = 0
    udp_packet.detSpec2 = 0
    udp_packet.detSpec3 = 0
    udp_packet.round_robin = 0
    udp_packet.detector_type = 1
    udp_packet.header_version = 2

    return udp_packet

def get_module_row_col(i_module):
    return i_module % 4, i_module // 4

def adjust_packet_for_module(udp_packet, i_module, packet_number):  
    udp_packet.row, udp_packet.column = get_module_row_col(i_module)
    udp_packet.packet_number = packet_number
    
@cache
def generate_data_for_packet(i_module, i_packet, bit_depth, n_rows_packet, n_cols_packet, n_packet_bytes):
    packet_bytes = 0
    i_pixel = 0
    
    for i_module_row in range(n_rows_packet):
        for i_module_col in range(n_cols_packet):
            pixel_value = 0
            packet_bytes |= pixel_value << int(i_pixel * bit_depth / 8)
            i_pixel += 1
    return packet_bytes.to_bytes(n_packet_bytes, 'little')


def generate_eg_udp_stream(output_address, start_udp_port, n_modules, rep_rate=0.1,
                           image_height=514, image_width=1030, n_images=None,
                           bit_depth=32):
    time_to_sleep = 1 / rep_rate

    udp_socket = socket(AF_INET, SOCK_DGRAM)
    udp_packet = init_eg_udp_packet(image_width)

    udp_packet_info = calculate_udp_packet_info(bit_depth)

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
        while udp_packet.frame_num < n_images:
            # First n-1 packets
            for i_packet in range(n_packets):
                udp_packet.packet_starting_row = udp_packet_info['packet_n_rows'] * i_packet

                for i_module in range(n_modules):                    
                    adjust_packet_for_module(udp_packet, i_module, i_packet)
                    bytes_data = ctypes.string_at(ctypes.byref(udp_packet), ctypes.sizeof(udp_packet))
                    # dummy_data = b'\x00' * 4096
                    udp_socket.sendto(bytes_data,#+dummy_data,
                                      (output_address, start_udp_port + i_module))

            iteration_end = time()
            iteration_time = iteration_end - iteration_start
            time_left_to_sleep = time_to_sleep - iteration_time

            # We do not sleep for less then 1ms.
            if time_left_to_sleep > 0.01:
                sleep(time_left_to_sleep)

            if iteration_end - print_start > PRINT_INTERVAL:
                print(f'Send all frames up to {udp_packet.frame_num}.')
                print_start = iteration_end

            udp_packet.frame_num += 1
            iteration_start = time()

    except KeyboardInterrupt:
        pass


def main():
    parser = argparse.ArgumentParser(description='Eiger udp stream generator.')
    parser.add_argument('detector_config_file', type=str, help='JSON file with detector configuration.')
    parser.add_argument('output_address', type=str, help='Address to send the UPD packets to.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)
    parser.add_argument('-n', '--n_images', type=int, help='Number of images to generate.', default=None)

    args = parser.parse_args()
    output_stream = args.output_address
    rep_rate = args.rep_rate
    n_images = args.n_images

    with open(args.detector_config_file, 'r') as input_file:
        config = json.load(input_file)

    start_udp_port = int(config['start_udp_port'])
    image_height = config['image_pixel_height']
    image_width = config['image_pixel_width']
    bit_depth = config['bit_depth']
    n_modules = config['n_modules']

    n_images_str = 'unlimited' if n_images is None else n_images
    print(f'Starting simulated Eiger with rep_rate {rep_rate} on {output_stream} '
          f'with start_udp_port {start_udp_port} and image_shape {(image_height, image_width)} '
          f'for {n_images_str} images and bit depth {bit_depth}.')

    generate_eg_udp_stream(output_stream, start_udp_port, n_modules, rep_rate, image_height, image_width, n_images, bit_depth)

    print('Stopping simulated stream.')


if __name__ == '__main__':
    main()
