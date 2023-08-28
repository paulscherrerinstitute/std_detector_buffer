from bsread.sender import Sender
import math
import struct
import numpy as np


def image(pulse_id):
    pulse_id = int(pulse_id)
    array = np.full((2048, 2048), pulse_id, dtype=np.int16)
    return array.to_bytes()


if __name__ == "__main__":
    generator = Sender(port=51111)
    generator.add_channel('data', image, metadata={'type': 'int16', 'shape': [2048, 2048]})
    generator.generate_stream(1000)
