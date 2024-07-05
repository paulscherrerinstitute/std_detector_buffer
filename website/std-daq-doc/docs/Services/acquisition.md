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

### GigaFRoST Detector Configuration

The `GigaFRoST` detector is characterized by a fixed configuration due to its consistent number of modules/sources. There are always 16 `udp_recv` services and an equal number of `converter` services, each labeled from `0` to `15`. These identifiers correlate with specific parts of the final image, aligned modulo 8.

Each `udp_recv` service is responsible for collecting packets from a designated single source and consolidating these into a complete frame. The `converter` services expand the `12-bit` encoded data into `16-bit` format. Afterwards the data is saved in the ring buffer according to the quadrant and datagram number as detailed in the [official documentation](https://hpdi.gitpages.psi.ch/gf_docs/gf_architecture.html#data-boards).

The synchronization of data from all 16 modules is managed to ensure ordered metadata stream output. Specifically, images labeled `0` and `7` are streamed simultaneously by the detector.

##### Services
* `std_udp_recv_gf`

  Command line options:
    ```text
    Usage: std_udp_recv_gf [--help] [--version] detector_json_filename module_id
    
    Positional arguments:
      detector_json_filename  - path to configuration file
      module_id               - module id (0-15) representing one of the sources from GigaFRoST detector
    ```
  Relevant config file parameters specific to receiver:
  * `start_udp_port` - Number of first port where `udp` data is sent from detector. We assume that the next connections will be incremental and in logical order of modules.
 
  Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).
* `std_data_convert_gf` - converts `12-bit` encoded data to `16-bit` encoding and puts data into correct location in the final image according to set `module_id`
  Command line options:
    ```text
    Usage: std_data_convert_gf [--help] [--version] detector_json_filename module_id
    
    Positional arguments:
      detector_json_filename  - path to configuration file
      module_id               - module id (0-15) representing one of the sources from GigaFRoST detector
    ```
  Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).
* `std_data_sync_module` - common synchronization service described [here](#std_data_sync_module). **GigaFRoST** requires `8` module configuration. This is due to the fact that single full image from the detector is created from `8` modules even if there are 16 connections. The other 8 connections provide in parallel another image, for details refer to [GigaFRoST documentation](http://hpdi.gitpages.psi.ch/gf_docs/gf_architecture.html).

### Jungfrau Detector Configuration

tbd

### Eiger Detector Configuration

tbd

### Common Detector Services

#### std_data_sync_module

Service is responsible for collecting information about finished processing of parts of the image from converting services. Internally it holds a queue of fixed size of metadata connected to given `image_id`. The images that were successfully received from all configured modules are sent with standard `protobuf` metadata [protocol](../Interfaces/protobuf.md) further in strictly increasing order. If there is an image that is incomplete (part of image was not received) it will be removed only when the queue is full - unblocking the rest of images to be sent out.

```text
Usage: std_data_sync_module [--help] [--version] detector_json_filename

Positional arguments:
      detector_json_filename  - path to configuration file
```
Relevant config file parameters specific to receiver:
* `n_modules` - Number of modules that require synchronization to complete single image
* `module_sync_queue_size` - size of the queue storing incomplete or out of order (not yet eligible for sending) images.

Common parameters affecting service can be found [here](../Interfaces/configfile.md#common-configuration-options).

## PCO Camera Data Acquisition

![PCO Acquisition](/img/pco_acquisition.svg)

## Buffered PCO Data Acquisition

![Redis Acquisition](/img/redis_acquisition.svg)
