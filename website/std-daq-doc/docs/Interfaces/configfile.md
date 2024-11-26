---
sidebar_position: 2
id: configfile
title: Configuration file
---

# `JSON` Configuration File

## Common configuration options

Options relevant for most/all services:

| Name                      | Type           | Description                                                                                                                                                                                                                                        |
|---------------------------|----------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `detector_name`           | Mandatory      | Name of deployment - used as identifier in logging, part of the name of `zmq` sockets and shared memory                                                                                                                                            |
| `detector_type`           | Mandatory      | Type of detector, one of: `gigafrost`, `eiger`, `pco`, `jungfrau-raw`, `jungfrau-converted`                                                                                                                                                        |
| `image_pixel_height`      | Mandatory      | Height of final image in pixels                                                                                                                                                                                                                    |
| `image_pixel_width`       | Mandatory      | Width of final image in pixels                                                                                                                                                                                                                     |
| `bit_depth`               | Mandatory      | Bit depth of the image                                                                                                                                                                                                                             |
| `log_level`               | Optional/Debug | Defaults to `info`. Sets the logging level for services - possible values: `debug`, `info`, `warning`, `error`, `off`                                                                                                                              |
| `stats_collection_period` | Optional       | Period in seconds for printing stats into `journald` that are shipped to `elastic`. Defaults to `10`. **Warning** too high frequency will affect the performance of the system                                                                     |
| `ram_buffer_gb`           | Optional       | RAM size allocated for image data buffer - if the size is not defined system allocates RAM for `1000` images as this is the minimum size for the system to work consistently. **Warning** user should not allocate all available RAM of the system |

## Configuration options affecting single service/service group

| Name                               | Type      | Description                                                                                                                                                                                                                                                                                                                                                                                                                               |
|------------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `n_modules`                        | Mandatory | Provides information about number of physical modules provided by the detector/camera. Used by converters/udp receivers to calculate where and how should they write data of the final image. Relevant for `module_positions` mapping                                                                                                                                                                                                     |
| `start_udp_port`                   | Mandatory | Address of first udp port where data from detector is incoming. The assumption is that the following ports will provide data for other modules in logical order according to detectors specification.                                                                                                                                                                                                                                     |
| `max_number_of_forwarders_spawned` | Optional  | Relevant when forwarding between servers is used. Defaults to `8`                                                                                                                                                                                                                                                                                                                                                                         |
| `use_all_forwarders`               | Optional  | Relevant when forwarding between servers is used. Defaults to `false`. Changes the way forwarding is deployed - This parameter should be `false` for large images where sending single image can be costly and images are divided in smaller chunks. In case of small images with high frequency this argument should be `true` as it allows to utilize all senders/receivers even if there is no need to divide the image to all of them |
| `sender_sends_full_images`         | Optional  | Relevant when forwarding between servers is used. Defaults to `false`. Forces senders to send full images. This affects the deployment of the system - metadata stream needs to be forwarded as well as metadata synchronizer is required to correctly synchronize the images on receiving server. Usually used only for small images with highest frequencies                                                                            |
| `module_sync_queue_size`           | Optional  | Relevant when module synchronizer. Defaults to `50`. Defines size of internal queue before the incomplete image is considered stale and gets dropped. The larger the queue the bigger latency is added to the system in case of sporadically missing data                                                                                                                                                                                 |
| `gpfs_block_size`                  | Optional  | Relevant for writing `hdf5` files. Defaults to `16777216`. `GPFS` block size in bytes defaulting to `16777216`. If this parameter is misconfigured it may affect performance of writing services as they allocate chunks of memory according to blocks in `GPFS`                                                                                                                                                                          |
| `module_positions`                 | Mandatory | Description of the position of each module in the final image - applicable only for `eiger` and `jungfrau` detectors. For others this parameters is ignored. It contains the list of 4 numbers lists which described `x,y` positions of the start point and end point of each module. The module can be rotated etc - the position needs to reflect it. Detailed usage is described for applicable detectors.                             |
| `live_stream_configs`              | Optional  | Configuration for `std_live_stream` service. Detailed description [here](../Services/interface.md#live-stream-interface).                                                                                                                                                                                                                                                                                                                 |
| `delay_filter_timeout`             | Optional  | Configuration for `std_delay_filter` service. Defines maximum delay in seconds the images will be delayed.                                                                                                                                                                                                                                                                                                                                |
|

## Examples

### GigaFRoST

Below are tested examples of configuration used for `GigaFRoST` detector on `daq-28/29` machines.

#### 2016x2016px @1.25kHz

```json
{
  "detector_name": "gf-teststand",
  "detector_type": "gigafrost",
  "n_modules": 8,
  "bit_depth": 16,
  "image_pixel_height": 2016,
  "image_pixel_width": 2016,
  "start_udp_port": 2000,
  "max_number_of_forwarders_spawned": 10,
  "use_all_forwarders": true,
  "module_sync_queue_size": 4096,
  "number_of_writers": 14,
  "module_positions": {},
  "live_stream_configs": {
    "tcp://129.129.95.111:20002": {
      "type": "periodic",
      "config": [
        1,
        5
      ]
    }
  }
}
```

#### 528x280px @21.5kHz

```json
{
  "detector_name": "gf-teststand",
  "detector_type": "gigafrost",
  "n_modules": 8,
  "bit_depth": 16,
  "image_pixel_height": 280,
  "image_pixel_width": 528,
  "start_udp_port": 2000,
  "max_number_of_forwarders_spawned": 10,
  "sender_sends_full_images": true,
  "use_all_forwarders": true,
  "module_sync_queue_size": 4096,
  "number_of_writers": 14,
  "module_positions": {}
}
```

#### 480x128px @40kHz

```json
{
  "detector_name": "gf-teststand",
  "detector_type": "gigafrost",
  "n_modules": 8,
  "bit_depth": 16,
  "image_pixel_height": 128,
  "image_pixel_width": 480,
  "start_udp_port": 2000,
  "max_number_of_forwarders_spawned": 10,
  "sender_sends_full_images": true,
  "use_all_forwarders": true,
  "module_sync_queue_size": 4096,
  "number_of_writers": 14,
  "module_positions": {}
}
```
