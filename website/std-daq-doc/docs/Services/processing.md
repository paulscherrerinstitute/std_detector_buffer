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

The compression reads data from `<detector-name>-image` ipc/ram buffer and provides output on `<detector-name>-blosc2` named channels. User can define number of threads used by the service for parallel execution and compression level `[0-9]` (higher value - better compression, slower processing).

```text
Usage: std_data_compress_blosc2 [--help] [--version] --threads VAR [--level VAR] detector_json_filename

Positional arguments:
detector_json_filename  - path to configuration file

Optional arguments:
-t, --threads           number of threads used for compression [required]
-l, --level             Compression level [nargs=0..1] [default: 5]
```

### h5bitshuffle_lz4

The compression reads data from `<detector-name>-image` ipc/ram buffer and provides output on `<detector-name>-h5bitshuffle-lz4` named channels. User can define number of threads used by the service for parallel execution and block size in bytes used for `bitshuffle` when `0` is specified algorithm chooses it automatically.

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

Filtering service for `GigaFRoST`. The service reads data from source with defined suffix e.g. `<detector-name>-image` and provides `metadata` stream named `<detector-name>-filter` with reduced data. All images with `store_image` flag set to `0` are filtered in [protobuf protocol](../Interfaces/protobuf.md#gfimagemetadata). There is a flag to forward all images for testing purposes.

```text
Usage: std_gf_filter [--help] [--version] [--source_suffix VAR] [--no_filter] detector_json_filename

Positional arguments:
detector_json_filename  - path to configuration file

Optional arguments:
-s, --source_suffix     suffix for ipc source for ram_buffer [nargs=0..1] [default: "image"]
-n, --no_filter         forward all images 
```
## Delayed Filtering
*TBD*
Filtering service that is delaying in time processing of the `zmq` processing chain of the images for given time in seconds or given number of images (whichever goes first).

```text
Usage: std_delay_filter [--help] [--version] [--source_suffix VAR] detector_json_filename

Positional arguments:
detector_json_filename

Optional arguments:
-h, --help              shows help message and exits
-v, --version           prints version information and exits
-s, --source_suffix     suffix for ipc and shared memory sources for ram_buffer [nargs=0..1] [default: "image"]
```

Relevant config file parameters relevant to this filter:
* `ram_buffer_gb` - size in GB of memory allocated as a ram buffer for fully assembled images - doesn't affect the size of udp receive buffer.
* `delay_filter_timeout` - maximum time after images are forwarded for further processing. If the images are not overflowing the ram buffer - this timeout is the trigger for continuation of processing.

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).
