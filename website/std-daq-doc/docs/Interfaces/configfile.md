---
sidebar_position: 2
id: configfile
title: Configuration file
---

# `JSON` Configuration File

## Common configuration options

Options relevant for most/all services:

| Name                      | Type           | Description                                                                                                                                                            |
|---------------------------|----------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `detector_name`           | Mandatory      | Name of deployment - used as identifier in logging, part of the name of `zmq` sockets and shared memory                                                                |
| `detector_type`           | Mandatory      | type of detector, one of: `gigafrost`, `eiger`, `pco`, `jungfrau-raw`, `jungfrau-converted`                                                                            |
| `image_pixel_height`      | Mandatory      | Height of final image in pixels                                                                                                                                        |
| `image_pixel_width`       | Mandatory      | Width of final image in pixels                                                                                                                                         |
| `bit_depth`               | Mandatory      | bit depth of the image                                                                                                                                                 |
| `log_level`               | Optional/Debug | Defaults to `info`. Sets the logging level for services - possible values: `debug`, `info`, `warning`, `error`, `off`                                                  |
| `stats_collection_period` | Optional       | Period in seconds for printing stats into journald that are shipped to elastic. Defaults to `10`. Warning too high frequency will affect the performance of the system |

## Configuration options affecting single service/service group

| Name                               | Type      | Description                                                                                                                                                                                                                                                                                                                                                                                                                               |
|------------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `n_modules`                        | Mandatory | Provides information about number of physical modules provided by the detector/camera. Used by converters/udp receivers to calculate where and how should they write data of the final image. Relevant for `module_positions` mapping                                                                                                                                                                                                     |
| `start_udp_port`                   | Mandatory | address of first udp port where data from detector is incoming. The assumption is that the following ports will provide data for other modules in logical order according to detectors specification.                                                                                                                                                                                                                                     |
| `max_number_of_forwarders_spawned` | Optional  | Relevant when forwarding between servers is used. Defaults to `8`                                                                                                                                                                                                                                                                                                                                                                         |
| `use_all_forwarders`               | Optional  | Relevant when forwarding between servers is used. Defaults to `false`. Changes the way forwarding is deployed - This parameter should be `false` for large images where sending single image can be costly and images are divided in smaller chunks. In case of small images with high frequency this argument should be `true` as it allows to utilize all senders/receivers even if there is no need to divide the image to all of them |
| `sender_sends_full_images`         | Optional  | Relevant when forwarding between servers is used. Defaults to `false`. Forces senders to send full images. This affects the deployment of the system - metadata stream needs to be forwarded as well as metadata synchronizer is required to correctly synchronize the images on receiving server. Usually used only for small images with highest frequencies                                                                            |
| `module_sync_queue_size`           | Optional  | Relevant when module synchronizer. Defaults to `50`. Defines size of internal queue before the incomplete image is considered stale and gets dropped. The larger the queue the bigger latency is added to the system in case of sporadically missing data                                                                                                                                                                                 |
| `gpfs_block_size`                  | Optional  | Relevant for writing `hdf5` files. Defaults to `16777216`. `GPFS` block size in bytes defaulting to `16777216`. If this parameter is misconfigured it may affect performance of writing services as they allocate chunks of memory according to blocks in `GPFS`                                                                                                                                                                          |
