---
sidebar_position: 8
id: livestream
title: Live Streaming
---

# Live Streaming

The `std_live_stream` service provides a real-time streaming interface. Users can connect to this service to receive live updates via a reduced data stream. Establishing a connection is facilitated through `TCP` using the `zmq` client and the `PUB/SUB` pattern, which allows subscribers to receive incoming data.

Data is provided in configured format - chosen at deployment. The available formats with specification:

- [array-1.0](https://github.com/paulscherrerinstitute/htypes/tree/master) - This is the default format, used for uncompressed data streams that require simple, straightforward streaming.
- [bsread](https://git.psi.ch/sf_daq/bsread_specification) - Suitable for compressed data streams that are utilized in more complex operational contexts
