from contextlib import contextmanager
from multiprocessing import shared_memory

import zmq
import zmq.asyncio


@contextmanager
def start_publisher_communication(zmq_context: zmq.asyncio.Context, config):
    socket = zmq_context.socket(zmq.PUB)
    socket.bind(f'ipc:///tmp/std-daq-{config.name}:{config.udp_port_base + config.id}')

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=True)

    yield memory.buf, socket

    memory.close()
    memory.unlink()


@contextmanager
def start_subscriber_communication(zmq_context: zmq.asyncio.Context, config):
    socket = zmq_context.socket(zmq.SUB)
    socket.connect(f'ipc:///tmp/std-daq-{config.name}:{config.udp_port_base + config.id}')
    socket.subscribe('')

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=False)

    yield memory.buf, socket

    memory.close()
    memory.unlink()
