---
sidebar_position: 1
id: development
title: Development
---

# Development

Please, follow the instructions for each of the components on their respective github repository.

The STD DAQ code is located at https://github.com/paulscherrerinstitute/sf_daq_buffer

The STD DAQ SERVICE code is located at https://github.com/paulscherrerinstitute/std_daq_service/

## EIGER

### Changes in detector's geometry and std-daq configuration
 
In case a detector configuration/geometry changes, one needs to prepare a new std-daq configuration file. In order to do that run the std-daq config generator, located in ```/etc/std_daq/git/std_detector_buffer/std_buffer/eiger```, and used as in:

    usage: std-daq_config_gen.py [-h] input_file output_file

    Std-DAQ Eiger configuration generator

    positional arguments:
      input_file   Path to the Eiger detector config file
      output_file  Path to the output file

    options:
      -h, --help   show this help message and exit
    
Please, take into account: 
- the detector's modules list need to be presented as a sequential list according to the detector's geometry geometry (`detsize`) and the list of boards (`hostname`). 
- the config generator script will create a module map that is used to define where each module's data are inserted into the final detector's image. If the `detsize` do not correspond to the appropriate geomtry and list of `hostname`, the final image will most likely not be assembled properly. 
- the `detsize` value in the detector config file do not take into account the gap pixels that are considered into the final image. Therefore, when checking the final image size, as found in the std-daq config file, it will be bigger because it includes the geometry gap pixels into it (which can be found in the Eiger's manual). For example, a `detsize` 3072 x 3072 will correspond to a final std-daq image size of  3264 x 3106 (image_pixel_height vs image_pixel_width). 
- The module map generated is presented in the final std-daq json configuration in the following format: `index: [x0, y0, x1, y1]`, as in:

```json
   ...
   "module_positions":{
      "0":[
         0,
         3008,
         513,
         3263
      ],
      "1":[
         516,
         3008,
         1029,
         3263
      ],
      "2":[
         0,
         3005,
         513,
         2750
      ],
    ...
```


