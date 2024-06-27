---
sidebar_position: 2
id: acquisition
title: Data Acquisition
---

# Data Acquisition

## Detector Data Acquisition

![Detector Acquisition](/img/acquisition_case1.svg)

The process of data acquisition for various detectors operates under a common framework, albeit with specific variations tailored to each detector type. These variations are detailed in sections dedicated to each detector.

Data is transmitted via UDP packets and handled by `udp_recv` services, which are scaled according to the number of detector modules in a deployment. The primary function of these services is to collect UDP packets from the detectors and assemble them into image frames. These frames are temporarily stored in a ring buffer within the system's RAM.

Each `udp_recv` service is directly linked to a `converter` service—there is a one-to-one correspondence between them. The converters are tasked with processing the raw frames by performing necessary transformations and ensuring that the data is correctly placed into another ring buffer designed for the final image assembly.

The converters then utilize `ZMQ push/pull` sockets to communicate the completion of their tasks to a `module_sync` service. This synchronization service ensures that each image frame is fully assembled and error-free. It also broadcasts metadata, using a `ZMQ pub` socket and `protobuf` protocol, which provides metadata stream to downstream systems. The image ids provided by synchronizer are strictly increasingly ordered.

#### GigaFRoST Detector Configuration

The `GigaFRoST` detector is characterized by a fixed configuration due to its consistent number of modules/sources. There are always 16 `udp_recv` services and an equal number of `converter` services, each labeled from `0` to `15`. These identifiers correlate with specific parts of the final image, aligned modulo 8.

Each `udp_recv` service is responsible for collecting packets from a designated single source and consolidating these into a complete frame. The `converter` services expand the `12-bit` encoded data into `16-bit` format. Afterwards the data is saved in the ring buffer according to the quadrant and datagram number as detailed in the [official documentation](https://hpdi.gitpages.psi.ch/gf_docs/gf_architecture.html#data-boards).

The synchronization of data from all 16 modules is managed to ensure ordered metadata stream output. Specifically, images labeled `0` and `7` are streamed simultaneously by the detector.

### Jungfrau Detector Configuration

tbd

### Eiger Detector Configuration

tbd

## PCO Camera Data Acquisition

![PCO Acquisition](/img/pco_acquisition.svg)

## Buffered PCO Data Acquisition

![Redis Acquisition](/img/redis_acquisition.svg)
