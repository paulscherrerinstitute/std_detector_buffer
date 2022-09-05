import argparse
from socket import socket, AF_INET, SOCK_DGRAM
from time import time, sleep

from std_buffer.gigafrost.data import GfUdpPacket, calculate_udp_packet_info

GF_N_MODULES = 8


def init_gf_udp_packet(image_width):
    udp_packet = GfUdpPacket()

    udp_packet.frame_index = 0
    udp_packet.protocol_id = 203
    udp_packet.scan_id = 42
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
    # No idea what this means - its just what we dumped when recording the detector.
    corr_mode = 5

    udp_packet.status_flags = 0
    udp_packet.status_flags |= (quadrant_id << 6)
    udp_packet.status_flags |= (link_id << 5)
    udp_packet.status_flags |= (corr_mode << 2)

    # The last bit in the 'quadrant_row' is the 'swap' bit.
    udp_packet.quadrant_rows = (image_height // 2) + swap


def generate_jf_udp_stream(output_address, start_udp_port, rep_rate=0.1,
                           image_height=2016, image_width=2016, n_images=None):
    time_to_sleep = 1 / rep_rate

    udp_socket = socket(AF_INET, SOCK_DGRAM)
    udp_packet = init_gf_udp_packet(image_width)

    udp_packet_info = calculate_udp_packet_info(image_height, image_width)
    n_packets = udp_packet_info['frame_n_packets']
    n_data_bytes = udp_packet_info['packet_n_data_bytes']

    data = bytearray(n_data_bytes)

    if not n_images:
        n_images = float('inf')

    try:
        iteration_start = time()

        while udp_packet.frame_index < n_images:

            # First n-1 packets
            for i_packet in range(n_packets-1):
                udp_packet.packet_starting_row = udp_packet_info['packet_n_rows'] * i_packet

                for i_module in range(GF_N_MODULES):
                    adjust_packet_for_module(udp_packet, i_module, image_height)
                    udp_socket.sendto(bytes(udp_packet)+data, (output_address, start_udp_port + i_module))
                    # Needed in case you cannot set rmem_max (docker)
                    sleep(0.001)

            # Last packet (may have different size and  number of lines)
            udp_packet.packet_starting_row += udp_packet_info['packet_n_rows'] * (n_packets-1)
            for i_module in range(GF_N_MODULES):
                adjust_packet_for_module(udp_packet, i_module, image_height)
                udp_socket.sendto(bytes(udp_packet)+data, (output_address, start_udp_port + i_module))

            iteration_end = time()
            time_left_to_sleep = max(0.0, time_to_sleep - (iteration_end - iteration_start))
            sleep(time_left_to_sleep)
            iteration_start = iteration_end

            print(f'Send frame {udp_packet.frame_index}.')
            udp_packet.frame_index += 1

    except KeyboardInterrupt:
        pass


def main():
    parser = argparse.ArgumentParser(description='Gigafrost udp stream generator.')
    parser.add_argument('output_address', type=str, help='Address to send the UPD packets to.')
    parser.add_argument('start_udp_port', type=int, help='First port on which to start the modules.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)
    parser.add_argument('--image_width', type=int, default=2016, help='Generated image width in pixels')
    parser.add_argument('--image_height', type=int, default=2016, help='Generated image width in pixels')
    parser.add_argument('-n', '--n_images', type=int, default=None, help='Number of images to generate.')

    args = parser.parse_args()
    output_stream = args.output_address
    start_udp_port = args.start_udp_port
    rep_rate = args.rep_rate
    image_height = args.image_height
    image_width = args.image_width
    n_images = args.n_images

    n_images_str = 'unlimited' if n_images is None else n_images
    print(f'Starting simulated GF with rep_rate {rep_rate} on {output_stream} '
          f'with start_udp_port {start_udp_port} and image_shape {(image_height, image_width)} '
          f'for {n_images_str} images.')

    generate_jf_udp_stream(output_stream, start_udp_port, rep_rate, image_height, image_width, n_images)

    print('Stopping simulated stream.')


if __name__ == '__main__':
    main()
