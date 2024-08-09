---
sidebar_position: 6
id: hdf5files
title: HDF5 Files
---

## HDF5 Files

To save data in the `hdf5` file format, you must use the [detector driver and writers](../Services/interface.md#hdf5-files-writing). 

Initially, raw files are created by each writer. Afterward, you can utilize the [REST interface](rest.md) to generate a file with virtual datasets. This file will aggregate all the data from the raw files and, if configured, also include metadata from the metadata file.

### Aggregated file structure

TBD

### Metadata 

For each type of detector there is unique set of metadata available for users to acquire.

#### GigaFRoST

Available fields taken directly fro `GigaFRoST` frame:

* image_id
* scan_id
* scan_time
* sync_time
* frame_timestamp
* exposure_time
