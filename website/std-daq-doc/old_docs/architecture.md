---
sidebar_position: 1
id: architecture
title: Architecture
---

# Overview

The overview of the STD DAQ is as follows:

![STD DAQ architecture](https://github.com/paulscherrerinstitute/sf_daq_buffer/raw/master/docs/sf_daq_buffer-overview.jpg)

:::tip

For a detailed component's description, please check [Components](../Components/std-udp-recv).

:::


## Scope

The scope should be as limited as possible and we should abstain from 
implementing additional features. We isolated the core functionality as the following:

- Receiving and assembling detector data.
- Writing detector data to HDF5 files.
- Live streaming detector data to external components.
- Provide logs and analytics for each component.

## Design goals

- Simplest thing that works.
  - Save time and iterate more quickly.
  - Less moving parts, easier to debug.
  - Avoid external libraries as much as possible.
- Start optimizing only when things break.
  - Many optimization possibilities, but not actually needed.
  - Makes possible to refactor and change code faster.
- Small debuggable and profileable processes.
  - Needs to be able to run on your local machine in a debugger.
  - Asses code performance without guessing.
- As little dependency between processes as possible.
  - Run only the process you want to test.
  - Write unit tests.

## Terminology

In order to unify the way we write code and talk about concepts the following 
terminology definitions should be followed:

- frame (data from a single module)
- image (assembled frames)
- start_pulse_id and stop_pulse_id is used to determine the 
inclusive range (both start and stop pulse_id are included) of pulses.
- pulse_id_step (how many pulses to skip between each image).
- GPFS buffer (detector buffering mechanism based on binary files on GPFS)
- detector_folder (root folder of the buffer for a specific detector on disk)
- module_folder (folder of one module inside the detector_folder)
- data_folder (folder where we group more buffer files based on pulse_id range)
- data_file (the files where the actual data is stored, inside data_folder)
