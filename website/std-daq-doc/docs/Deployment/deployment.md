---
sidebar_position: 1
id: deployment
title: Deployment
---

# Deployment

Release of `std-daq` is done automatically into [packages](packages.md). The installation/deployment process can be done in multiple ways depending on the users. `std_daq` is tested against a set of ansible deployments in [this repository](https://git.psi.ch/std_daq_configs/sls/poc_fast_deploy) and the ansible role [psi.std_daq_deploy](https://git.psi.ch/controls-ci/psi.std_daq_deploy) is developed alongside.

## Example deployments

Tested example deployments can be found in https://git.psi.ch/std_daq_configs/sls/poc_fast_deploy repository.

### GigaFRoST deployment for 2 servers explained:

Current test deployment for `GigaFRoST` utilizing 2 servers can be found [here](https://git.psi.ch/std_daq_configs/sls/poc_fast_deploy/-/blob/master/tomcat.gf1.h5_compression.yml?ref_type=heads). Below is detailed explanation for this setup.


#### First server (daq-28) aka receiving data from detector
1. Software version
   ```yaml
   - std_daq_sw_version: 0.13.34
   ...
    ```
2. List of deployment - currently there is only 1 deployment of `std_daq` in this setup
   ```yaml
   deployments:
     - config_file: gf1_480_128.json
       microservices:
         - ...
    ```
   1. `std_udp_recv_gf` - GigaFRoST receiver services running for all 0-15 connections
      ```yaml
         - prog_name: std_udp_recv_gf
           instances:
             - { cpus: [0] , params: [0] }
             - { cpus: [1] , params: [1] }
             - ...
             - { cpus: [7] , params: [7] }
             - { cpus: [24], params: [8] }
             - ...
             - { cpus: [31], params: [15] }
      ```
      Each service uses exclusively only 1 dedicated cpu, different parts can be connected to different cars/NUMAs.
   2. `std_data_convert_gf` - data converters corresponding to each receiver
      ```yaml
            - prog_name: std_data_convert_gf
              instances:
                - { cpus: [8] , params: [0] }
                - { cpus: [9] , params: [1] }
                - ...
                - { cpus: [23], params: [15] }
      ```
      Each service uses exclusively only 1 dedicated cpu
   3. `std_data_sync_module` - single instance of module synchronizer to synchronize converted and assembled images in order
      ```yaml
            - prog_name: std_data_sync_module
              instances:
                - { cpus: [43-47], params: [] }
      ```
      Synchronizer takes up more cores as it receives and processes in parallel messages from all 16 converters.
   4. `std_metadata_stream` - single instance of output metadata stream in case of sending with high frequencies (40kHZ) small images between the servers
      ```yaml
            - prog_name: std_metadata_stream
              instances:
                - { cpus: [42], params: ["'tcp://192.168.10.224:10100'"] }
      ```
      Only one core needed - not heavily utilized, the parameter is the address of the zmq `PUB/SUB` socket to which one may connect to receive the metadata stream.
   5. `std_stream_send` - 10 sender services forwarding data in one of the configured modes between machines
      ```yaml
            - prog_name: std_stream_send
              instances:
                - { cpus: [32], params: ["'tcp://192.168.10.224:10000'", 0] }
                - ...
                - { cpus: [41], params: ["'tcp://192.168.10.224:10009'", 9] }

      ```

#### Second server (daq-29) aka writing data to GPFS server
**WARNING** Software version and configuration file **need** to be configured to the same version as for the first server.
1. Microservices deployed on this machine:
   1. `std_stream_receive` - 10 receiver each corresponding to sender on `daq-28` server with corresponding `tcp` address configured for sender.
      ```yaml
         - prog_name: std_stream_receive
           instances:
             - { cpus: [3] , params: ["tcp://192.168.10.224:10000", 0] }
             - ...
             - { cpus: [12], params: ["tcp://192.168.10.224:10009", 9] }

      ```
      Each service uses exclusively only 1 dedicated cpu, different parts can be connected to different cars/NUMAs.
   2. `std_data_sync_stream` and `std_data_sync_metadata` - two synchronizers spawned for different mode of communication (large images vs small images). Depending on the configuration file one of them shuts down on startup.
      ```yaml
            - prog_name: std_data_sync_stream
              instances:
                - { cpus: [15-18], params: [] }
            - prog_name: std_data_sync_metadata
              instances:
                - { cpus: [15-18], params: ['tcp://192.168.10.224:10100'] }
      ```
      Synchronizer takes up more cores as it receives and processes in parallel from all receivers.
   3. `std_det_driver` - detector driver responsible for controlling file writers
      ```yaml
            - prog_name: std_det_driver
              instances:
                - { cpus: [19-21], params: ['-s filter'] }
      ```
      Requires more cores to handle in parallel user requests and communication with writers. `filter` is a suffix of data channel that it takes to provide data to writers.
   4. `std_det_writer` - 14 writers spawned to serve file writing used in parallel by detector driver.
      ```yaml
            - prog_name: std_det_writer
              instances:
                - { cpus: [36], params: [0] }
                - ...
                - { cpus: [25], params: [13] }

      ```
      Each uses only 1 core - the number of utilized at a time writers depends on the configuration in configuration file.
   5. `std_live_stream` - 2 live streams spawned with different parameters for presentation purposes
      ```yaml
           - prog_name: std_live_stream
             instances:
               - { cpus: [22], params: ['tcp://129.129.95.38:20000', '-b 2:555'] }
               - { cpus: [22], params: ['tcp://129.129.95.38:20001', '-p 4:10'] }
      ```
      First parameter is the output stream for the `array-1.0` interface sent over `zmq` second is the mode of operation `-b` - batch sending 2 images every 555 images, `-p` - periodic sending 4 images with 10Hz frequency.
