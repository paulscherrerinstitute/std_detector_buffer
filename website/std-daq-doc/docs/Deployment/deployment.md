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
      Converter takes up more cores as it receives and processes in parallel messages from all 16 converters.
   4. `std_metadata_stream` - single instance of output metadata stream in case of sending with high frequencies (40kHZ) small images between the servers
      ```yaml
            - prog_name: std_metadata_stream
              instances:
                - { cpus: [42], params: ["'tcp://192.168.10.224:10100'"] }
      ```
      Only one core needed - not heavily utilized, the parameter is the address of the zmq `PUB/SUB` socket to which one may connect to receive the metadata stream.
