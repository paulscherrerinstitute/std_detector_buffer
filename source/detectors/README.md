# Detectors

## Integration of a new detector

In this chapter we will lay out one procedure that could be followed to integrate a new detector into the standard daq. 
The integration can be done on 3 levels, and this guide should cover all of them.

### Architectural overview
![detectors overview](../docs/detectors_overview.jpg)

The data first need to be received from the network. This is done in the **std\_udp\_recv** service. This service 
receives an input stream (of UDP packets, usually) from a single detector module and assembles the packets into 
detector frames (frame in this context means the image part coming from a single detector module). The frame and its 
metadata are then stored into the FrameBuffer (RamBuffer instance containing frames). In addition, the service also 
publishes all the assembled frames on a ZMQ socket (sending only the assembled image_id).

After having received the same image_id via ZMQ from all active receivers, the **std\_data\_converter** assembles the 
individual frames into images, taking care of data conversion if needed, calculates the ImageMetadata fields from the
received FrameMetadata-s, and stores the result into the ImageBuffer (RamBuffer instance containing images).

From the ImageBuffer on we have standard services that can be used to save images to files, stream them to other 
machines and make them available to other external processes.

You can integrate a detector into the standard_daq on 3 levels:

- You can write your own UDP receivers.
- You can write your own data converters.
- You can push data directly into the ImageBuffer.

### Receiving data from a detector - writing your own UDP receivers

This step usually needs to be done only for new detectors. The currently supported detectors are:

- Jungfrau
- Eiger
- Gigafrost

Since the receivers have no logic inside (accumulate packets and construct frames) they can be reused in almost 
any scenario. Probably the only exception is when the performance of the standard UDP receivers is not good enough, 
and you need a custom solution. Here we will assume you have a new detector, and you need to do everything from scratch.

The procedure below can be followed as a step-by-step guide that includes:

- Dumping the UDP stream coming from the detector to files.
- Writing a packet analyzer to characterize the UDP packets and verify that the provided detector documentation is correct.
- Write a detector simulator that generates the correct UDP packets and validate them by comparing them with the dumped 
packets from the detector.
- Implement the first version of your UDP receivers.
- Implement integration tests with the detector simulator and udp receivers.
- Performance test your UDP receivers on the real detector. 

#### Dumping the detector UDP stream

You need exclusive access to the detector and to a server the detector can stream data to. In this case the performance 
does not matter, so it could be also your personal PC if the detector can be configured to output data slowly.

The goal is to collect the stream of at least a 2 images acquisition. Usually the transition between images has 
edge cases, so we want to be sure we can analyze them later in the process. We will also need to collect streams in 
different operational modes or ROI configurations (if the detector supports this). In general, we need a sample of all 
possible edge cases the detector has when outputting data.

If the detector is outputting a UDP stream of packets, then you can use the **std\_udp\_tools\_dump** to save the raw 
packets in files:

```bash
mkdir acquisition_name
cd acquisition_name 
std_udp_tools_dump detector_config.json
```

The tool will create 1 file per UDP port (1 detector module, usually) in the folder where it was run and dump all the 
received UDP packets into it. You can stop the dumping by pressing any key. Dump different acquisitions in different files.
The detector config you need to pass to the tool is the std_daq detector config file 
we use for all detector related services. The JSON file should look like (Gigafrost example):

```json
{
  "detector_name": "GF2",
  "detector_type": "gigafrost",
  "n_modules": 8,
  "bit_depth": 16,
  "image_pixel_height": 2016,
  "image_pixel_width": 2016,
  "start_udp_port": 2000
}
```

In this specific case, we have 8 modules and the start_udp_port is 2000. This means that after running std_udp_tools_dump
we will get 8 files: 2000.dat, 2001.dat ... 2007.dat. We can now transfer this files to our development machine - we 
will analyze them with the packet analyzer we will write in the next step.

#### Writing a packet analyzer

You will need the UDP stream dumps from the detector. Check the previous chapter for details.

The goal is to verify that the provided detector documentation matches reality, or in case you don't have much 
documentation to work with, to find out how the detector constructs and streams out data. At the end of this step it 
should be clear to you how the protocol works and how to simulate the detector stream in all edge cases.

There will be no direct instructions in this chapter as how and what you write in your packet analyzer is highly 
dependent on the detector you are implementing and on the documentation you have available. As an example I can 
provide a link to the packet analyzer that was used to verify the Gigafrost protocol:

- [Gigafrost packet analyzer](https://github.com/paulscherrerinstitute/std_detector_buffer/blob/master/testing/gigafrost/analyze_udp_dump.py)

#### Writing a detector simulator

You will need an intimate knowledge of the detector produced streams under all edge cases. The packet analyzer, 
together with the detector documentation, should have clarified that.

The goal is to have the ability to simulate the detector under any operational configuration. This is needed for 
improving development speed, minimizing the need to have access to the detector and allows for the possibility of 
having automated testing - both unit and later integration testing.

There will be no direct instructions on how to write a detector simulator, but the Gigafrost example should give you a good 
starting point:

- [Gigafrost detector simulator](https://github.com/paulscherrerinstitute/std_detector_buffer/blob/master/std_buffer/gigafrost/udp_gf_sim.py)


### Converting frames into images


### Pushing data into the image buffer
