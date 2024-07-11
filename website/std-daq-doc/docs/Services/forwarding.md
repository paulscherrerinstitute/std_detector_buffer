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

## Forwarding 1

![Forwarding 1](/img/forwarding_1.svg)

tbd

## Forwarding 2

![Forwarding 2](/img/forwarding_2.svg)

tbd
