---
sidebar_position: 5
id: processing
title: Data Processing
---

# Data Processing

Processing services are designed to seamlessly modify existing data stream and providing modified data in exactly the same manner. The `metadata` is received/sent via [protobuf protocol](../Interfaces/protobuf.md) and image data is provided via shared memory according to [ring buffer](../Interfaces/ringbuffer.md) specification.

## Data Compression

![Processing 1](/img/processing_1.svg)

There are 2 available compression algorithms for that users can choose from:
- `blosc2` - https://github.com/Blosc/c-blosc2
- `bishuffle_lz4` - https://github.com/kiyo-masui/bitshuffle it is modified for compatibility with **hdf5** files filter 

The compression updates `metadata` of the stream [protobuf protocol](../Interfaces/protobuf.md#imagemetadata). Namely field - `compression` with type `ImageMetadataCompression` and possible values `none`, `h5bitshuffle_lz4`, `blosc2`

### blosc2

The compression reads data from `<detector-name>-image` ipc/ram buffer and provides output on `<detector-name>-blosc2>` named channels. User can define number of threads used by the service for parallel execution and compression level `[0-9]` (higher value - better compression, slower processing).

```text
Usage: std_data_compress_blosc2 [--help] [--version] --threads VAR [--level VAR] detector_json_filename

Positional arguments:
detector_json_filename  - path to configuration file

Optional arguments:
-t, --threads           number of threads used for compression [required]
-l, --level             Compression level [nargs=0..1] [default: 5]
```

### h5bitshuffle_lz4

The compression reads data from `<detector-name>-image` ipc/ram buffer and provides output on `<detector-name>-h5bitshuffle-lz4>` named channels. User can define number of threads used by the service for parallel execution and block size in bytes used for `bitshuffle` when `0` is specified algorithm chooses it automatically.

```text
Usage: std_data_compress_h5bitshuffle_lz4 [--help] [--version] --threads VAR [--block_size VAR] detector_json_filename

Positional arguments:
detector_json_filename  - path to configuration file

Optional arguments:
  -t, --threads           number of threads used for compression [required]
  -b, --block_size        block size in bytes [nargs=0..1] [default: 0]
```

## GigaFRoST Filtering

![Processing 2](/img/processing_2.svg)

tbd
