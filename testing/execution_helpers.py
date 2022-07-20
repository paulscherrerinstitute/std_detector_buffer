import shlex
import subprocess
from pathlib import Path
from contextlib import contextmanager


def executable(name) -> Path:
    binary_path = Path(__file__).parent.parent.absolute()
    for file in binary_path.rglob(name):
        if file.is_file():
            return file
    assert False


def build_command(executable_name: str, *args) -> str:
    return f'{executable(executable_name)} ' + ' '.join([str(arg) for arg in args])


@contextmanager
def run_command_in_parallel(command: str):
    args = shlex.split(command)
    process = None
    try:
        process = subprocess.Popen(args=args, cwd=Path(__file__).parent)
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
