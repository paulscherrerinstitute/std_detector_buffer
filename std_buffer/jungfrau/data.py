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
    def __init__(self):
        self.id = 1
        self.name = f'jungfrau-{self.id}'
        self.udp_port_base = 50020
        self.data_bytes_per_packet = 8192 * 128
        self.packets_per_frame = 128
        self.slots = 10  # should be 1000 but for testing purposes 10 is enough
        self.buffer_size = self.data_bytes_per_packet * self.slots


class JungfrauConfigConverter:
    def __init__(self):
        udp = JungfrauConfigUdp()
        self.id = udp.id
        self.converter_index = 3
        self.name = f'jungfrau-{self.id}-converted-{self.converter_index}'
        self.data_bytes_per_packet = udp.data_bytes_per_packet * 2
        self.udp_port_base = udp.udp_port_base
        self.slots = udp.slots
        self.buffer_size = self.data_bytes_per_packet * self.slots
