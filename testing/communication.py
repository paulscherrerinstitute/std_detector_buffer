from contextlib import contextmanager
from multiprocessing import shared_memory


@contextmanager
def open_shared_memory(name: str, size: int, create: bool = False):
    input_memory = shared_memory.SharedMemory(name=name, size=size, create=create)

    yield input_memory.buf

    input_memory.close()
    input_memory.unlink()
