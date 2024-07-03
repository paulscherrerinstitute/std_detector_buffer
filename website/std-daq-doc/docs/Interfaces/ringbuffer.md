---
sidebar_position: 4
id: ringbuffer
title: RAM Ring Buffer
---

## RAM Ring Buffer

RAM ring buffer is a part of **internal** communication protocol between services. It holds image data corresponding to the [metadata sent over zmq](protobuf.md). This shared memory buffer communication is designed for inter-process communication or fast data storage accessible by multiple processes on the same machine. The naming convention for buffers is following `<detector-name>-service/purpose` e.g. `gf-image` for fully synchronized and assembled image or `gf-blosc2` for compressed version of image.

### Assumptions:

- each ring buffer has only 1 service that writes to it but can have multiple reading clients
- processing done by clients is faster than incoming images from detector (as ring buffer will be overwritten with new images)
- position of given image in buffer is calculated based on `imaged_id % n_slots` where `n_slots = 1000` - which is subject to change in future.
- Size of each element of the buffer is calculated at the startup of service based on the size and bit depth of the
  image.

Size calculations and implementation can be found within the repository. The number of available slots is now fixed - subject to change in future.

**THIS IS INTERNAL INTERFACE** - developers will not consider external users during development process. Not adhering to assumptions may break the functionality of services - e.g. modifying the memory that other service is filling may result in corrupted images saved by system.
