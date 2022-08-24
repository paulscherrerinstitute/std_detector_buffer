from ctypes import Structure, c_uint32, c_uint64, c_uint16, c_uint8

GF_MAX_PAYLOAD = 7400


class GfUdpPacket(Structure):
    _pack_ = 1
    _fields_ = [("protocol_id", c_uint8),
                ("quadrant_row_length_in_blocks", c_uint8),
                ("quadrant_rows", c_uint8),
                ("status_flags", c_uint8),
                ("scan_id", c_uint32),
                ("frame_index", c_uint32),
                ("image_status_flags", c_uint16),
                ("packet_starting_row", c_uint16),
                ("image_timing", c_uint64),
                ("sync_time", c_uint32),
                ("scan_time", c_uint32)]

    def __str__(self):
        return f"frame_index: {self.frame_index}; " \
               f"quadrant_rows: {(self.quadrant_rows >>1) << 1}; " \
               f"quadrant_row_length_in_blocks: {self.quadrant_row_length_in_blocks}; " \
               f"packet_starting_row: {self.packet_starting_row}; "


class GFFrame(Structure):
    _pack_ = 1
    _fields_ = [("frame_index", c_uint64),
                ("n_missing_packets", c_uint64),
                ("scan_id", c_uint32),
                ("size_x", c_uint32),
                ("size_y", c_uint32),
                ("scan_time", c_uint32),
                ("sync_time", c_uint32),
                ("frame_timestamp", c_uint64),
                ("exposure_time", c_uint64),
                ("swapped_rows", c_uint32),
                ("link_id", c_uint32),
                ("corr_mode", c_uint32),
                ("quadrant_id", c_uint32),
                ("rpf", c_uint32),
                ("do_not_store", c_uint32)]

    def __str__(self):
        return f"link_id: {self.link_id} " \
               f"quadrant_id: {self.quadrant_id}"


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
