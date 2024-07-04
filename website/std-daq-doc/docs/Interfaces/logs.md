---
sidebar_position: 7
id: logs
title: Logging
---

# Logging

Each service running as part of `std_daq` has implemented common logging mechanism based on `spdlog` framework. The logs are provided with common format

```text
[date][service][version][log_type] message
```

All of these messages are printed directly to standard output, which in case of `systemd` services is logged. Additionally, using `filebeat` service logs are shipped to `elastic`. Based on these logs [std_daq_monitoring](../Monitoring/intro.md) works.

Example logs:

```text
[07-03 15:50:52.977][std_det_writer][v0.13.33][info] detector=gf-teststand,source=image,id=2,n_written_images=0,avg_buffer_write_us=0,max_buffer_write_us=0,avg_throughput=0.00 13384997578250107
[07-03 15:50:55.928][std_data_sync_metadata][v0.13.33][info] detector=gf-teststand,processed_times=0,repetition_rate_hz=0.00,n_corrupted_images=0,queue=0 13385000529215367
[07-03 15:50:55.930][std_gf_filter][v0.13.33][info] detector=gf-teststand,source=image,processed_times=0,repetition_rate_hz=0.00,forwarded_images=0 13385000531051339
[07-03 15:50:56.930][std_live_stream][v0.13.33][info] detector=gf-teststand,port=20001,type=array10,source=image,processed_times=0,repetition_rate_hz=0.00 13385001531304642
[07-03 15:50:56.931][std_live_stream][v0.13.33][info] detector=gf-teststand,port=20000,type=array10,source=image,processed_times=0,repetition_rate_hz=0.00 13385001531377136
[07-03 15:50:58.922][std_stream_receive][v0.13.33][info] detector=gf-teststand,image_part=6,processed_times=0,repetition_rate_hz=0.00,zmq_receive_fails=0,images_missed=0 13385003523232350
[07-03 15:50:58.923][std_stream_receive][v0.13.33][info] detector=gf-teststand,image_part=7,processed_times=0,repetition_rate_hz=0.00,zmq_receive_fails=0,images_missed=0 13385003524204785

```
#### Configuration

There are following parameters affecting logging and stats collection:

- `log_level` - defaults to `info`. Sets the logging level for services - possible values: `debug`, `info`, `warning`, `error`, `off` 
- `stats_collection_period` - Period in seconds for printing stats into journald that are shipped to elastic. defaults to `10`. **Warning** too high frequency will affect the performance of the system.
