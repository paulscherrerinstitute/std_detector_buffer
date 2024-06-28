---
sidebar_position: 2
id: configfile
title: Configuration file
---

# `JSON` Jonfiguration File

## Common configuration options

Options relevant for most/all services:

| Name                      | Type           | Description                                                                                                                                                            |
|---------------------------|----------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `image_pixel_height`      | Mandatory      | Height of final image in pixels                                                                                                                                        |
| `image_pixel_width`       | Mandatory      | Width of final image in pixels                                                                                                                                         |
| `log_level`               | Optional/Debug | Defaults to `info`. Sets the logging level for services - possible values: `debug`, `info`, `warning`, `error`, `off`                                                  |
| `stats_collection_period` | Optional       | Period in seconds for printing stats into journald that are shipped to elastic. Defaults to `10`. Warning too high frequency will affect the performance of the system |
