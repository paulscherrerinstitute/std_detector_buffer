---
sidebar_position: 4
id: interface
title: Data Presentation/Interface
---

# Data Presentation/Writing Interface

Elements described here provide functionality to communicate with users/other services.

## Live Stream Interface

![Presentation 2](/img/presentation_2.svg)

tbd

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
