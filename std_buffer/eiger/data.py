class EigerConfigUdp:
    id = 0
    name = f'EG1M-{id}'
    udp_port_base = 50020
    data_bytes_per_packet = 512 * 256 * 2
    # packets_per_frame = 128
    slots = 10  # should be 1000 but for testing purposes 10 is enough
    buffer_size = data_bytes_per_packet * slots


class EigerConfigConverter:
    id = EigerConfigUdp.id
    converter_index = 0
    name = 'EG1M-image'
    socket_name = 'EG1M-sync'
    data_bytes_per_packet = (4 * EigerConfigUdp.data_bytes_per_packet) + (12 * 514) + (4 * 1024)
    udp_port_base = EigerConfigUdp.udp_port_base
    slots = EigerConfigUdp.slots
    buffer_size = data_bytes_per_packet * slots
