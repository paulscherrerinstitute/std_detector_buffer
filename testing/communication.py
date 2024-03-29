from contextlib import contextmanager
from multiprocessing import shared_memory

import os
import zmq
import zmq.asyncio


@contextmanager
def start_publisher_communication(zmq_context: zmq.asyncio.Context, config,
                                  socket_type: zmq.SocketType = zmq.SocketType.PUB):
    socket = zmq_context.socket(socket_type)
    try:
        socket.bind(f'ipc:///tmp/{config.socket_name}')
    except AttributeError:
        socket.bind(f'ipc:///tmp/{config.name}')

    file = '/dev/shm/' + config.name
    if os.path.exists(file):
        os.remove(file)

    memory = shared_memory.SharedMemory(name=config.name, size=config.buffer_size, create=True)

    yield memory.buf, socket

    memory.close()
    memory.unlink()


@contextmanager
def start_subscriber_communication(zmq_context: zmq.asyncio.Context, config,
                                   socket_type: zmq.SocketType = zmq.SocketType.SUB):
    socket = zmq_context.socket(socket_type)
    socket.connect(f'ipc:///tmp/{config.name}')
    if socket_type == zmq.SocketType.SUB:
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
