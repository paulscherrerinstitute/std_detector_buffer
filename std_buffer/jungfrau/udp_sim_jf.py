import argparse
from socket import socket, AF_INET, SOCK_DGRAM
from time import time, sleep
import json

from std_buffer.jungfrau.data import UdpPacket

PRINT_INTERVAL = 10


def init_jf_udp_packet() -> UdpPacket:
    udp_packet = UdpPacket()

    udp_packet.framenum = 0
    udp_packet.exptime = 0
    udp_packet.packetnum = 0
    udp_packet.bunchid = 0
    udp_packet.timestamp = 0
    udp_packet.moduleID = 0
    udp_packet.row = 0
    udp_packet.column = 0
    udp_packet.zCoord = 0
    udp_packet.debug = 0
    udp_packet.roundRobin = 0
    udp_packet.detectortype = 0
    udp_packet.headerVersion = 0

    return udp_packet


def generate_data_for_packet(i_module, i_packet, n_packet_bytes):
    packet_bytes = 0
    i_pixel = 0

    for pixel in range(int(n_packet_bytes / 2)):
        pixel_value = pixel
        pixel_value |= i_module << 14
        pixel_value |= (i_packet % 16) << 10
        packet_bytes |= pixel_value << (i_pixel * 16)
        i_pixel += 1

    return packet_bytes.to_bytes(n_packet_bytes, 'little')


def generate_jf_udp_stream(output_address, start_udp_port, rep_rate=0.1, n_modules=1, n_images=None):
    time_to_sleep = 1 / rep_rate

    udp_socket = socket(AF_INET, SOCK_DGRAM)
    udp_packet = init_jf_udp_packet()

    n_packets = 128
    n_data_bytes = 8192

    if not n_images:
        n_images = float('inf')

    try:
        iteration_start = time()
        print_start = iteration_start

        while udp_packet.framenum < n_images:
            for i_packet in range(n_packets):
                for i_module in range(n_modules):
                    udp_packet.moduleID = i_module
                    data = generate_data_for_packet(i_module, i_packet, n_packet_bytes=n_data_bytes)

                    udp_socket.sendto(bytes(udp_packet)+data,
                                      (output_address, start_udp_port + i_module))

            iteration_end = time()
            iteration_time = iteration_end - iteration_start
            time_left_to_sleep = time_to_sleep - iteration_time

            # We do not sleep for less then 1ms.
            if time_left_to_sleep > 0.01:
                sleep(time_left_to_sleep)

            if iteration_end - print_start > PRINT_INTERVAL:
                print(f'Send all frames up to {udp_packet.framenum}.')
                print_start = iteration_end

            udp_packet.framenum += 1
            udp_packet.bunchid = udp_packet.framenum
            iteration_start = time()

    except KeyboardInterrupt:
        pass


def main():
    parser = argparse.ArgumentParser(description='Gigafrost udp stream generator.')
    parser.add_argument('detector_config_file', type=str, help='JSON file with detector configuration.')
    parser.add_argument('output_address', type=str, help='Address to send the UPD packets to.')
    parser.add_argument('-r', '--rep_rate', type=int, help='Repetition rate of the stream.', default=10)
    parser.add_argument('-n', '--n_images', type=int, default=None, help='Number of images to generate.')
    parser.add_argument('-m', '--n_modules', type=int, default=None, help='Number of modules to generate.')

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

    generate_jf_udp_stream(output_stream, start_udp_port, rep_rate, args.n_modules, n_images)
    print('Stopping simulated stream.')


if __name__ == '__main__':
    main()
