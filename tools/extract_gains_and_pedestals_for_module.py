#!/usr/bin/python3

import argparse
import sys
from pathlib import Path

import h5py as h5py

parser = argparse.ArgumentParser(description='Extract from h5 file data values for single module')
parser.add_argument('filename', type=str, help='input h5 file with all module gains and pedestals values')
parser.add_argument('module_id', type=int, help='id of module to be extracted')

arguments = parser.parse_args(sys.argv[1:])
print(f'Extracting from {arguments.filename=} {arguments.module_id=}')

with h5py.File(arguments.filename, 'r') as f:
    if f['gains'].shape[0] < arguments.module_id:
        raise ValueError(f'Provided {arguments.module_id=} is greater than dimension in h5 file!')
    if arguments.module_id <= 0:
        raise ValueError(f'Provided {arguments.module_id=} must be greater than 0!')

    original_file = Path(arguments.filename)
    new_file = original_file.stem + f'-module-{arguments.module_id}' + original_file.suffix
    with h5py.File(new_file, 'w') as out:
        for field in ['gains', 'gainsRMS']:
            values = f[field]
            out_vals = out.create_dataset(name=field, shape=values.shape[1:], dtype=values.dtype)
            out_vals[:] = values[arguments.module_id][:]

        out.create_dataset(name='pixel_mask', data=f['pixel_mask'])
