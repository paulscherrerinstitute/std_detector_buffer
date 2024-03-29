---
sidebar_position: 2
id: std-udp-sync
title: std-udp-sync
---

# std-udp-sync

[std-udp-sync](https://github.com/paulscherrerinstitute/sf_daq_buffer/tree/eiger/std-udp-sync) is the component that verifies if the frames from all the modules have arrived and are synchronized. It creates multiple ZMQ receiving connections to receive the frames' arrival confirmations, as many as the number of receivers (std-udp-recv), and sends out the uniqueID to external components.

---

**NOTE**

uniqueID can be be different for the different facilities/instances of std-daq. For example, in SwissFEL std-daq uses the PulseID, while at SLS Eiger (9M cSAXS) uses the ImageID.

---

## Overview

std-udp-sync is used for signaling that all the frames with the same UniqueID have arrived, are synchronized and ready to be assembled.
In case that the frames are not synchronized, identified by different UniqueID's, the std-udp-sync throws a runtime error exception.

It receives the detector's configuration file path as argument, which is parsed and necessary to define, for example, the total number of modules.

## ZMQ receiving

Each ZMQ stream is coming from a separate std-udp-recv. This means that we have as many connections as we have modules in a detector.

Messages are received in PUB/SUB mode and contain the UniqueID.

## ZMQ sending

Once all sockets are synchronized and confirmed, the UniqueID is sent out to external components.

## std-udp-sync statistics

std-udp-sync logs statistics regarding the number of processed/corrupted images in the following InfluxDB format, as follows:

```bash
std_udp_sync,detector_name=cSAXS.EG01V01 n_processed_images=1i,n_sync_lost_images=0i,repetition_rate=0i 1627033283173988905
```
