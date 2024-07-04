---
sidebar_position: 8
id: livestream
title: Live Streaming
---

# Live Streaming

When users spawn `std_live_stream` services they provide an interface that one can connect to and receive live updates from reduced stream. The connection can be established on `tcp` with `zmq` client using `PUB/SUB` pattern to subscribe to incoming data.

Data is provided in configured format - chosen at deployment. The available formats with specification:

- [array-1.0](https://github.com/paulscherrerinstitute/htypes/tree/master) - for uncompressed data preferred for simple streaming. It is also **default** stream
- [bsread](https://git.psi.ch/sf_daq/bsread_specification) - for compressed data and more demanding operations
