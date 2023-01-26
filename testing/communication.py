from contextlib import contextmanager
from multiprocessing import shared_memory

import zmq
import zmq.asyncio


@contextmanager
def start_publisher_communication(zmq_context: zmq.asyncio.Context, config):
    socket = zmq_context.socket(zmq.PUB)
    try:
        socket.bind(f'ipc:///tmp/{config.socket_name}')
    except AttributeError:
        socket.bind(f'ipc:///tmp/{config.name}')

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=True)

    yield memory.buf, socket

    memory.close()
    memory.unlink()


@contextmanager
def start_subscriber_communication(zmq_context: zmq.asyncio.Context, config):
    socket = zmq_context.socket(zmq.SUB)
    socket.connect(f'ipc:///tmp/{config.name}')
    socket.subscribe('')

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=False)

    yield memory.buf, socket

    memory.close()
    memory.unlink()


@contextmanager
def start_pull_communication(zmq_context: zmq.asyncio.Context, config):
    socket = zmq_context.socket(zmq.PULL)
    try:
        socket.bind(f'ipc:///tmp/{config.socket_name}')
    except AttributeError:
        socket.bind(f'ipc:///tmp/{config.name}')

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=False)

    yield memory.buf, socket

    memory.close()
    memory.unlink()
