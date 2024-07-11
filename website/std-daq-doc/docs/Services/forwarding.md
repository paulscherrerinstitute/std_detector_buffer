---
sidebar_position: 3
id: forwarding
title: Data Forwarding
---

# Data Forwarding

The services detailed in this section ensure the seamless transfer of [metadata streams](../Interfaces/protobuf.md) and image data held in [RAM buffers](../Interfaces/ringbuffer.md) between servers. This data retains the format provided by the [data acquisition](acquisition.md) services, allowing for similar processing methods.

Data forwarding within our system accommodates two distinct use cases, each optimized for specific image sizes and frequencies:

1. Small to High Frequency with Large Images:
   - Images are segmented and transmitted via multiple sender/receiver pairs.
   - A synchronizer reassembles the segments, ensuring the data stream remains synchronized and intact.

2. High to Very High Frequencies with Small Images:
   - Images are transmitted in their entirety one at a time through each sender/receiver pair.
   - A concurrent metadata stream facilitates the synchronization of these images, arranging them in strict order. This process includes flags for any missing images and minimizes latency.

Forwarding is done over TCP using `zmq` protocol.

## Forwarding 1

![Forwarding 1](/img/forwarding_1.svg)

This setup for forwarding is intended for large images or slower frequencies (as it adds lower latency than second way of forwarding of images). The number of pairs sender/receiver will be utilized depending on the image size (the smaller the image the less pairs used). The image will be divided equally between `max_number_of_forwarders_spawned` for maximum size of an image for given detector.

Configuration file options intended for this mode:
* `use_all_forwarders` - false
* `sender_sends_full_images` - false

*Services used in this model are described below*.

### std_stream_send

```text
Usage: std_stream_send [--help] [--version] detector_json_filename stream_address image_part

Positional arguments:
  detector_json_filename  path to configuration file
  stream_address          address to bind the output stream 
  image_part              0..7 responsible for sending n-th part of image 
```

Relevant config file parameters specific to sender/receiver pair:
* `use_all_forwarders` - Enforces all configured forwarder pairs to be used.
* `sender_sends_full_images` - Configures the mode where each pair is handling full image instead of part (requires different synchronization service)
* `max_number_of_forwarders_spawned` - number of forwarder pairs services installed on the server available for usage

### std_stream_receive

Service receives the incoming data from sender service and informs synchronizer about available image or part of it.

```text
Usage: std_stream_receive [--help] [--version] detector_json_filename stream_address image_part

Positional arguments:
  detector_json_filename  path to configuration file
  stream_address          address to bind input stream 
  image_part              0..7 responsible for sending n-th part of image 
```

### std_data_sync_stream

Synchronizes images incoming from receiver services in this mode. If the `sender_sends_full_images` is set it finishes immediately assuming that other kind of synchronization is used. Provides as an output `PUB/SUB` metadata stream using `zmq` with `-image` suffix. Exactly the same as outcome of data acquisition services.

```text
Usage: std_data_sync_stream [--help] [--version] detector_json_filename

Positional arguments:
  detector_json_filename  path to configuration file
```

## Forwarding 2

![Forwarding 2](/img/forwarding_2.svg)

tbd
