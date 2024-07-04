---
sidebar_position: 3
id: Protobuf
title: Protobuf Metadata Protocol
---

## ImageMetadata

`ImageMetadata` is the main message containing metadata sent between services via `zmq` protocol. It combines different types of image metadata into a single structure. The protocol uses `ipc` communication with `PUB/SUB` type of communication based on `zmq` protocol implementation. All addresses start with `ipc:///tmp/`.

### Fields
- `image_id` (`uint64`): Unique identifier for the image.
- `height` (`uint64`): Height of the image in pixels.
- `width` (`uint64`): Width of the image in pixels.
- `size` (`uint64`): Total size of the image data.

### Enums
- `dtype` (`ImageMetadataDtype`): Data type of the image - one of:
  `unknown` , `uint8` , `uint16` , `uint32` , `uint64` , `int8` , `int16` , `int32` , `int64` , `float16` , `float32` , `float64`
- `status` (`ImageMetadataStatus`): Status of the image - one of: `undefined`, `good_image` , `missing_packets` , `id_missmatch`
- `compression` (`ImageMetadataCompression`): Compression type used - `none` , `h5bitshuffle_lz4` , `blosc2`

### Optional detector specific metadata:

image_metadata (`oneof`):

- `gf` [`GFImageMetadata`](#gfimagemetadata): Metadata for GF images.
- `jf` [`JFImageMetadata`](#jfimagemetadata): Metadata for JF images.
- `eg` [`EGImageMetadata`](#egimagemetadata): Metadata for EG images.
- `pco` [`PcoImageMetadata`](#pcoimagemetadata): Metadata for PCO images.

## GFImageMetadata

`GFImageMetadata` includes metadata specific to GigaFRoST detector images.

### Fields
- `scan_id` (uint32): Identifier for the scan.
- `scan_time` (uint32): Time of the scan.
- `sync_time` (uint32): Synchronization time.
- `frame_timestamp` (uint64): Timestamp of the frame.
- `exposure_time` (uint64): Exposure time.
- `store_image` (bool): Flag to determine if the image should be stored.

## JFImageMetadata

`JFImageMetadata` pertains specifically to Jungfrau detector image data acquisition records.

### Fields
- `daq_rec` (uint64): Data acquisition record identifier for images.

## EGImageMetadata

`EGImageMetadata` is related to Eiger detector image settings.

### Fields
- `exptime` (uint64): Exposure time for images.

## PcoImageMetadata

`PcoImageMetadata` contains metadata specific to PCO camera images.

### Fields
- `global_timestamp_sec` (uint64): Global timestamp in seconds.
- `global_timestamp_ns` (uint64): Global timestamp in nanoseconds.
- `bsread_name` (string): Name associated with the BSREAD data stream.
