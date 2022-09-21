#!/usr/bin/python3

import argparse
import json
import sys
from pathlib import Path

import h5py as h5py


def process_module(filename: str, module_id: int, detector_data: dict):
    with h5py.File(filename, 'r') as f:
        original_file = Path(filename)
        new_file = original_file.stem + f'-module-{module_id}' + original_file.suffix
        width = detector_data['image_pixel_width']
        height = detector_data['image_pixel_height']

        with h5py.File(new_file, 'w') as out:
            for field in ['gains', 'gainsRMS']:
                values = f[field]
                out_vals = out.create_dataset(name=field, shape=(3, width, height), dtype=values.dtype)
                for gain in range(3):
                    out_vals[gain] = values[gain][(module_id - 1) * width:module_id * width]

            out.create_dataset(name='pixel_mask', shape=(width, height), dtype=f['pixel_mask'].dtype,
                               data=f['pixel_mask'][(module_id - 1) * width:module_id * width])


def main():
    parser = argparse.ArgumentParser(description='Extract from h5 file data values for single module')
    parser.add_argument('detector_file', type=str,
                        help='input h5 file with all module gains and pedestals values')
    parser.add_argument('h5_file', type=str, help='input h5 file with all module gains and pedestals values')
    parser.add_argument('-m', '--module_id', type=int,
                        help='id of module to be extracted, indexed from 1, (0 = all modules)',
                        default=0)

    arguments = parser.parse_args(sys.argv[1:])
    data = json.loads(Path(arguments.detector_file).read_text())

    if arguments.module_id < 0 or arguments.module_id > data['n_modules']:
        raise ValueError(f'Provided {arguments.module_id=} must be greater be within range [0,{data["n_modules"]}]')

    if arguments.module_id != 0:
        process_module(arguments.h5_file, arguments.module_id, data)
    else:
        for i in range(data['n_modules']):
            process_module(arguments.h5_file, i + 1, data)


if __name__ == '__main__':
    main()



