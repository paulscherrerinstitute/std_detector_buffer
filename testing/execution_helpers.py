import shlex
import subprocess
from pathlib import Path


def executable() -> Path:
    binary_path = Path(__file__).parent.parent.absolute()
    for file in binary_path.rglob("std_data_convert"):
        if file.is_file():
            return file
    assert False


def build_command(detector_json_filename: str, gains_and_pedestals: str, module_id: int) -> str:
    testing_path = Path(__file__).parent.absolute() / 'test_files'
    return f'{executable()} {testing_path / detector_json_filename} {testing_path / gains_and_pedestals} {module_id} '


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
