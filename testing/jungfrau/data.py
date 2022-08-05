from ctypes import Structure, c_uint8, c_uint32, c_uint16, c_uint64, c_double


class UdpPacket(Structure):
    _pack_ = 1
    _fields_ = [("framenum", c_uint64),
                ("exptime", c_uint32),
                ("packetnum", c_uint32),
                ("bunchid", c_double),
                ("timestamp", c_uint64),
                ("moduleID", c_uint16),
                ("row", c_uint16),
                ("column", c_uint16),
                ("zCoord", c_uint16),
                ("debug", c_uint32),
                ("roundRobin", c_uint16),
                ("detectortype", c_uint8),
                ("headerVersion", c_uint8),
                ("data", c_uint16 * 4096)]

    def __str__(self):
        return f"framenum: {self.framenum}; " \
               f"packetnum: {self.packetnum};"


class Frame(Structure):
    _pack_ = 1
    _fields_ = [("pulse_id", c_uint64),
                ("n_missing_packets", c_uint64),
                ("id", c_uint64),
                ("frame_index", c_uint64),
                ("daq_rec", c_uint64),
                ("module_id", c_uint64),
                ("data", c_uint8 * (64 - 16 - 32))]

    def __str__(self):
        return f"pulse_id: {self.pulse_id} " \
               f"module_id: {self.module_id}"


class JungfrauConfigUdp:
    id = 1
    name = f'jungfrau-{id}'
    udp_port_base = 50020
    meta_bytes_per_packet = 48
    data_bytes_per_packet = 8192 * 128
    bytes_per_packet = meta_bytes_per_packet + data_bytes_per_packet
    packets_per_frame = 128
    slots = 10  # should be 1000 but for testing purposes 10 is enough
    buffer_size = bytes_per_packet * slots


class JungfrauConfigConverter:
    id = JungfrauConfigUdp.id
    name = f'jungfrau-{id}-converted'
    data_bytes_per_packet = JungfrauConfigUdp.data_bytes_per_packet * 2
    udp_port_base = JungfrauConfigUdp.udp_port_base
    slots = JungfrauConfigUdp.slots
    meta_bytes_per_packet = JungfrauConfigUdp.meta_bytes_per_packet
    bytes_per_packet = meta_bytes_per_packet + data_bytes_per_packet
    buffer_size = bytes_per_packet * slots
