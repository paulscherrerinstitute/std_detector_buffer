from ctypes import Structure, c_uint16, c_uint64

import zmq
import numpy as np


class ImageMetadata(Structure):
    _pack_ = 1
    _fields_ = [("id", c_uint64),
                ("height", c_uint64),
                ("width", c_uint64),
                ("dtype", c_uint16),
                ("status", c_uint16),
                ("source_id", c_uint16)]

    dtype_mapping = {
        1: 'uint8', 2: 'uint16', 4: 'uint32', 8: 'uint64',
        11: 'int8', 12: 'int16', 14: 'int32', 18: 'int64',
        22: 'float16', 24: 'float32', 28: 'float64'
    }

    status_mapping = {
        0: 'good_image', 1: 'missing_packets', 2: 'id_missmatch'
    }

    def get_dtype_description(self):
        return self.dtype_mapping[self.dtype]

    def get_status_description(self):
        return self.status_mapping[self.status]

    def __str__(self):
        return f"id: {self.id}; height: {self.height}; width: {self.width}; " \
               f"dtype: {self.dtype}; status: {self.status}; source_id: {self.source_id};"


class StdStreamRecvBinary(object):
    def __init__(self, input_stream_address, recv_timeout_ms=500):
        self.input_stream_address = input_stream_address
        self.recv_timeout_ms = recv_timeout_ms

        self.ctx = None
        self.receiver = None

    def connect(self):
        if self.ctx is not None:
            raise RuntimeError("Socket already connected.")

        self.ctx = zmq.Context()
        self.receiver = self.ctx.socket(zmq.SUB)
        self.receiver.RCVTIMEO = self.recv_timeout_ms
        self.receiver.connect(self.input_stream_address)
        self.receiver.subscribe("")

    def disconnect(self):
        try:
            self.receiver.close()
            self.ctx.term()
        finally:
            pass

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    @staticmethod
    def _deserializer(multipart_bytes):
        meta = ImageMetadata.from_buffer_copy(multipart_bytes[0])
        data = np.frombuffer(multipart_bytes[1], dtype=meta.get_dtype_description()).reshape((meta.height, meta.width))

        return meta, data

    def recv(self):
        return self.receiver.recv_serialized(self._deserializer)
