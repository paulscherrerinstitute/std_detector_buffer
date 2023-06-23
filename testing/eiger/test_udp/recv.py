import pickle
import socket

import numpy as np

from std_buffer.eiger.data import EgUdpPacket

# Define frame header data type
frame_header_dt = np.dtype([
    ("Frame Number", "u8"),
    ("SubFrame Number/ExpLength", "u4"),
    ("Packet Number", "u4"),
    ("Bunch ID", "u8"),
    ("Timestamp", "u8"),
    ("Module Id", "u2"),
    ("Row", "u2"),
    ("Column", "u2"),
    ("Reserved", "u2"),
    ("Debug", "u4"),
    ("Round Robin Number", "u2"),
    ("Detector Type", "u1"),
    ("Header Version", "u1"),
])

ip = "0.0.0.0"
# ports = list(range(50020, 50021, 1))
# sockets = [socket.socket(socket.AF_INET, socket.SOCK_DGRAM) for i in range(len(ports))]
socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
socket.bind((ip, 50020))
# for s, p in zip(sockets, ports):
#     print(p)
#     s.bind((ip, p))

counter=0
while True:
    # for s in sockets:
    data, address = socket.recvfrom(4096+48)
    h = np.frombuffer(data, count=1, dtype=frame_header_dt)[0]
    print(h)

socket.close()