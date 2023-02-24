import shlex
import shutil
import subprocess
import time
from pathlib import Path
from contextlib import contextmanager

import numpy as np
import zmq
import zmq.asyncio

import testing.std_daq.image_metadata_pb2 as daq_proto


def executable(name) -> Path:
    binary_path = Path(__file__).parent.parent.absolute()
    for file in binary_path.rglob(name):
        if file.is_file():
            return file

    filename = shutil.which(name)
    if filename:
        return filename

    assert False


def build_command(executable_name: str, *args) -> str:
    return f'{executable(executable_name)} ' + ' '.join([str(arg) for arg in args])


@contextmanager
def run_command_in_parallel(command: str, sleep=1):
    args = shlex.split(command)
    process = None
    try:
        process = subprocess.Popen(args=args, cwd=Path(__file__).parent)
        time.sleep(sleep)
        yield
    finally:
        if process:
            process.terminate()


def run_command(command: str, env=None) -> (int, str, str):
    try:
        args = shlex.split(command)

        process_result = subprocess.run(args=args,
                                        cwd=Path(__file__).parent,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        env=env)

        if not process_result:
            raise RuntimeError('ERROR: command failed')

        s = process_result.stdout.decode('utf-8') if process_result.stdout is not None else ''
        e = process_result.stderr.decode('utf-8') if process_result.stderr is not None else ''

        return process_result.returncode, s, e

    except Exception as err:
        return -1, str(err), ''


def push_to_buffer(input_buffer, data):
    sent_data = np.ndarray((len(data),), dtype='b', buffer=input_buffer)
    sent_data[:] = np.frombuffer(data, dtype='b')
    return sent_data


async def send_receive(pub_socket: zmq.asyncio.Socket, slot: int, sub_socket: zmq.asyncio.Socket):
    msg = sub_socket.recv()
    time.sleep(0.1)
    await pub_socket.send(np.array([slot], dtype='i8'))
    time.sleep(0.1)
    return await msg


async def send_receive_proto(pub_socket: zmq.asyncio.Socket, slot: int, sub_socket: zmq.asyncio.Socket):
    msg = sub_socket.recv()
    time.sleep(0.1)
    metadata = daq_proto.ImageMetadata()
    metadata.image_id = slot
    await pub_socket.send(metadata.SerializeToString())
    time.sleep(0.1)
    data = await msg
    metadata.ParseFromString(data)
    return metadata
