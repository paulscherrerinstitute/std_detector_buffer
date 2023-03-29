import argparse
import socket
import numpy as np
import sys

frame_header_dt = np.dtype(
	[
		("Frame Number", "u8"),
		("Sub Frame Number (expLength)", "u4"),
		("Packet Number", "u4"),
		("detSpec1", "u8"),
		("Timestamp", "u8"),
		("Module Id", "u2"),
		("Row", "u2"),
		("Column", "u2"),
		("detSpec2", "u2"),
		("detSpec3", "u4"),
		("detSpec4", "u2"),
		("Detector Type", "u1"),
		("Header Version", "u1"),
	]
)

parser = argparse.ArgumentParser(description='Eiger udp packet streamdebugger.')
parser.add_argument('-i', '--ip', type=str, help='Network interface ip', default='10.30.30.211')
parser.add_argument('-p', '--port', type=int, default=50020, help='Initial port number.')
parser.add_argument('-n', '--number_of_recvs', type=int, default=1, help='Number of receivers to spawn.')

args = parser.parse_args()

ip = args.ip
init_port = args.port
n_recvs = args.number_of_recvs
ports = list(range(init_port, init_port+n_recvs, 1))
sockets = [socket.socket(socket.AF_INET, socket.SOCK_DGRAM) for i in range(len(ports))]

for s, p in zip(sockets, ports):
	print(ip,p)
	s.bind((ip,p))

while True:
	for s in sockets:
		data, address = s.recvfrom(4144)
		h = np.frombuffer(data, count=1, dtype=frame_header_dt)[0]
		print(h)