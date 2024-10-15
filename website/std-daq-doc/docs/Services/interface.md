---
sidebar_position: 4
id: interface
title: Data Presentation/Interface
---

# Data Presentation/Writing Interface

Elements described here provide functionality to communicate with users/other services.

## Live Stream Interface

![Presentation 2](/img/presentation_2.svg)


`std_live_stream` service provides a possibility of providing live stream to user in one of following ways:

- stream `Y` consecutive images with `N` frequency.
- stream `Y` consecutive images every `N` images.

The provided stream is a based on `tcp` connection with `zmq` socket of `PUB/SUB` type. Multiple users may subscribe to any of the live streams. There are 2 protocols available for the users

- [array-1.0](https://github.com/paulscherrerinstitute/htypes/tree/master) - for uncompressed data preferred for simple streaming. It is also **default** stream
- [bsread](https://git.psi.ch/sf_daq/bsread_specification) - for compressed data and more demanding operations

The live stream can be connected to any source adhering to [internal metadata/ring buffer protocol](#basic-metadata--ram-buffer-interface).

#### Config file settings

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).

- `live_stream_configs` - is a dictionary defining the configurations for all `live streams` allowing users to modify the settings without the need to redeploy the service.

  Example config setting up `batch` sending of `2` images every `555`, `forwarding` full stream and `periodic` sending of `1` image with `5 Hz` frequency.

  ```json
  "live_stream_configs": {
    "tcp://129.129.95.111:20000": {
      "type": "batch",
      "config": [2, 555]
    },
    "tcp://129.129.95.111:20001": {
      "type": "forward",
      "config": []
    },
    "tcp://129.129.95.111:20002": {
      "type": "periodic",
      "config": [1, 5]
    }
  }
  ```

#### Usage

```text
Usage: std_live_stream [--help] [--version] [--source_suffix VAR] [--type VAR] detector_json_filename stream_address

Positional arguments:
  detector_json_filename  path to configuration file
  stream_address          address to bind the output stream 

Optional arguments:
  -s, --source_suffix     suffix for ipc source and ram_buffer - default "image" [nargs=0..1] [default: "image"]
  -t, --type              choose the type: 'bsread' or 'array-1.0' [nargs=0..1] [default: <not representable>]
```

## HDF5 Files Writing

![Presentation 3](/img/presentation_3.svg)

This is a set of services responsible for saving images requested by the user to files in `hdf5` format. The sources of the metadata and image data can be specified separately as long as they adhere to common [internal protocol](#basic-metadata--ram-buffer-interface).

The part of the system consists of 2 types of services. Single `driver` responsible for communication with the user and controlling of `N` writers responsible for parallel writing of the data to end point (usually `gpfs`). Additionally, one can configure special metadata driver specific for each detector type. This service will create separate file consisting of metadata for all images.

### `std_det_driver` Service

The user can communicate and send requests to `driver` service using `WebSocket` protocol described in detail [here](../Interfaces/driverwebsocket.md). It will communicate via `zmq` channel with each `writer` and provide needed information to create files, write images, close files. This is internal interface and a pair `driver <-> writer` are not meant to be used separately or communicate with any other services in the system.

#### Config file settings

- `number_of_writers` - it will communicate and divide the incoming `metadata` stream between given number of drivers. The drivers with `id % number_of_writers` will be addressed. The images will be divided in similar way `image_id % number_of_writers`.

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).

#### Usage

```text
Usage: std_det_driver [--help] [--version] [--source_suffix VAR] [--port VAR] [--with_metadata_writer] detector_json_filename

Positional arguments:
  detector_json_filename  path to configuration file

Optional arguments:
  -s, --source_suffix         suffix for ipc source for metadata [nargs=0..1] [default: "image"]
  -p, --port                  websocket listening port [nargs=0..1] [default: 8080]
  -m, --with_metadata_writer  Whether writer for metadata is accessible/used 
```

### `std_det_writer` Services

There can be multiple `writer` services spawned on the deployment and connected to single `driver`. `writer` service will write files as described in detail [here](../Interfaces/hdf5files.md). If `driver` uses only subset of `writers` in current deployment the writers unused will automatically shutdown. 

#### Config file settings

- `number_of_writers` - if a `writer_id` is greater of equal of this value - the writer will shutdown at startup.
- `gpfs_block_size` - `GPFS` block size in bytes defaulting to `16777216`. If this parameter is misconfigured it may affect performance of writing services as they allocate chunks of memory according to blocks in `GPFS`.

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).

#### Usage 

```text
Usage: std_det_writer [--help] [--version] [--source_suffix VAR] detector_json_filename writer_id

Positional arguments:
  detector_json_filename  path to configuration file
  writer_id               id number of the driver

Optional arguments:
  -s, --source_suffix     suffix for shared memory source for ram_buffer - default "image" [nargs=0..1] [default: "image"]
```

### `std_det_meta_writer_gf` Service

Single service that can be spawned to save image metadata specific for `GigaFRoST` detector.

#### Config file settings

- `gpfs_block_size` - `GPFS` block size in bytes defaulting to `16777216`. If this parameter is misconfigured it may affect performance of writing services as they allocate chunks of memory according to blocks in `GPFS`.

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).

#### Usage

```text
Usage: std_det_meta_writer_gf [--help] [--version] detector_json_filename

Positional arguments:
  detector_json_filename  path to configuration file
```

## ImageBuffer API

![Presentation 4](/img/presentation_4.svg)

tbd

## Basic Metadata + Ram Buffer Interface

![Presentation 1](/img/presentation_1.svg)

**WARNING** this interface is considered as internal communication interface and changes will be done without consideration of "third-party" clients (any service not part of `std_daq`).

User can connect to standard interface used in communication between the services at any point. The interface consists of 2 parts. `Metadata` send via `zmq` socket on `ipc` with `PUB/SUB` model of communication and `ring buffer` containing image data within shared memory. The first point where user can connect is after [data acquisition](acquisition.md) - the communication channels are called `<detector-name>-image`. Other access points are available after [processing services](processing.md).

Detailed description of interfaces can be found:
- [metadata](../Interfaces/protobuf.md)
- [ring buffer](../Interfaces/ringbuffer.md)
