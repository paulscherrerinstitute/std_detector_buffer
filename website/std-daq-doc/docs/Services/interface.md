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

The live stream can be connected to any source adhering to [internal metadata/ring buffer protocol](#basic-metadata--ram-buffer-interface),

```text
Usage: std_live_stream [--help] [--version] [--source_suffix VAR] [--type VAR] [--forward] [--periodic VAR] [--batch VAR] detector_json_filename stream_address

Positional arguments:
  detector_json_filename  path to configuration file
  stream_address          address to bind the output stream 

Optional arguments:
  -s, --source_suffix     suffix for ipc source for ram_buffer - default "image" [nargs=0..1] [default: "image"]
  -t, --type              choose the type: 'bsread' or 'array-1.0' [nargs=0..1] [default: <not representable>]
  -f, --forward           forward all images 
  -p, --periodic          periodically send Y images every N Hz (format: Y:N) 
  -b, --batch             send Y images every N images (format: Y:N) 

```

## HDF5 Files Writing

![Presentation 3](/img/presentation_3.svg)

tbd

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
