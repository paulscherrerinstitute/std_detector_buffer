import struct
from pathlib import Path

from std_buffer.gigafrost.data import GfUdpPacket, gf_udp_packet_to_frame

GF_N_MODULES = 8


def read_next_packet(input_file):
    # uint64_t with the number of bytes per packet.
    try:
        n_bytes_to_read = struct.unpack('Q', input_file.read(8))[0]
    except TypeError:
        print("There seems to be nothing left in the file?")
        raise

    data = input_file.read(n_bytes_to_read)
    packet = GfUdpPacket.from_buffer_copy(data)

    return packet, n_bytes_to_read


def main(dumped_folder, generated_folder):

    dumped_folder_abs = Path(__file__).parent.absolute() / 'udp_dumps' / f'{dumped_folder}'
    generated_folder_abs = Path(__file__).parent.absolute() / 'udp_dumps' / f'{generated_folder}'

    dumped_files = []
    generated_files = []
    for i_module in range(GF_N_MODULES):
        dumped_files.append(open(f'{dumped_folder_abs}/200{i_module}.dat', mode='rb'))
        generated_files.append(open(f'{generated_folder_abs}/200{i_module}.dat', mode='rb'))

    try:
        i_packet = 0
        while True:
            packets_are_different = False
            for i_module in range(GF_N_MODULES):
                dumped_packet, dumped_packet_size = read_next_packet(dumped_files[i_module])
                generated_packet, generated_packet_size = read_next_packet(generated_files[i_module])

                if dumped_packet_size != generated_packet_size or dumped_packet != generated_packet:
                    print(f'Difference in module {i_module}:')
                    print(f'Dumped ({dumped_packet_size} bytes): {dumped_packet}')
                    print(f'Generated: ({generated_packet_size} bytes): {generated_packet}')
                    print()

                    packets_are_different = True

            if packets_are_different:
                pass
                raise Exception(f'Packet missmatch detected in i_packet {i_packet}.')

            i_packet += 1

    except struct.error as e:
        # EOF usually
        pass

    print("Comparison completed. Packet streams are identical.")

    for file in dumped_files:
        file.close()

    for file in generated_files:
        file.close()


if __name__ == '__main__':
    dumped_folder = '2016_2016'
    generated_folder = f'gen_{dumped_folder}'

    main(dumped_folder=dumped_folder,
         generated_folder=generated_folder)

