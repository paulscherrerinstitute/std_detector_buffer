---
sidebar_position: 9
id: rest
title: Config & Test Rest Interface
---

## Config & Test Rest Interface

The Rest Interface, implemented in Python using FastAPI, provides endpoints for managing the configuration of std-daq as well as testing various components of the system. The configuration file is stored locally on the servers running std-daq, and updates via the API trigger an automatic restart of the std-daq microservices to adopt the new settings.

**Important:**  
A misconfigured camera or detector configuration file can result in malfunctioning of the std-daq. The configuration file is defined per deployment, and local changes made via the RestAPI are temporary. Persistent changes must be submitted via the [config repo](https://git.psi.ch/std_daq_configs).

The Rest Interface is dockerized and configured through ansible playbooks. In multi-server deployments, such as for GigaFRoST, the primary rest interface propagates changes to the secondary server to ensure both run with identical configurations.

---

## RestAPI Endpoints

### Config Endpoints

#### `/api/config/get`

- **Method:** GET
- **Description:** Returns the current configuration file used by std-daq.
- **Parameters:**
    - `user`: Username to be logged.
- **Usage Example:**

  ```bash
  curl -X GET "http://xbl-daq-29:5000/api/config/get?user=username"
  ```
  
  ```python
  import getpass
  import json
  import requests
    
  response = requests.get('http://xbl-daq-29:5000/api/config/get', params={'user': getpass.getuser()})
  response.raise_for_status()
  config = response.json()
  print(json.dumps(config, indent=4))
  ```
  
#### `/api/config/set`

- **Method:** POST
- **Description:** Updates the configuration file for std-daq. The entire configuration must be provided and valid according to the JSON schema.
- **Parameters:**
    - `user`: Username to be logged.
    - `content`: json file with config
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/config/set?user=username" \
  -H "Content-Type: application/json" \
  -d '{
  "detector_name": "gf-teststand",
  "detector_type": "gigafrost",
  "n_modules": 8,
  "bit_depth": 16,
  "image_pixel_height": 2016,
  "image_pixel_width": 2016,
  "start_udp_port": 2000,
  "writer_user_id": 18600,
  "submodule_info": {},
  "max_number_of_forwarders_spawned": 10,
  "use_all_forwarders": true,
  "module_sync_queue_size": 4096,
  "module_positions": {}
  }'
  ```

  ```python
  import getpass
  import json
  import requests

  headers = {'Content-Type': 'application/json'}
  payload = {'user': getpass.getuser()}
  new_config = {
      "detector_name": "gf-teststand",
      "detector_type": "gigafrost",
      "n_modules": 8,
      "bit_depth": 16,
      "image_pixel_height": 2016,
      "image_pixel_width": 2016,
      "start_udp_port": 2000,
      "writer_user_id": 18600,
      "submodule_info": {},
      "max_number_of_forwarders_spawned": 10,
      "use_all_forwarders": True,
      "module_sync_queue_size": 4096,
      "module_positions": {}
  }
  response = requests.post('http://xbl-daq-29:5000/api/config/set', params=payload, data=json.dumps(new_config), headers=headers)
  response.raise_for_status()
  print(response.json())
  ```


### H5 Endpoints

These endpoints help in managing and analyzing HDF5 files generated by std-daq.


#### `/api/h5/read_metadata`
- **Method:** GET
- **Description:** Reads metadata from an HDF5 file.
- **Parameters:**
    - `filename`: Full path of the HDF5 file.
- **Usage Example:**

  ```bash
  curl -G "http://xbl-daq-29:5000/api/h5/read_metadata" --data-urlencode "filename=/gpfs/test/test-beamline/interleaved_api_virtual.h5"
  ```

#### `/api/h5/get_metadata_status`
- **Method:** GET
- **Description:** Provides details on datasets, shapes, types, and metadata within an HDF5 file.
- **Parameters:**
    - `filename`: Full path of the HDF5 file.
- **Usage Example:**

  ```bash
  curl -G "http://xbl-daq-29:5000/api/h5/get_metadata_status" --data-urlencode "filename=/gpfs/test/test-beamline/interleaved_api_virtual.h5"
  ```

#### `/api/h5/create_interleaved_vds`
- **Method:** POST
- **Description:** Creates an interleaved virtual dataset (VDS) from multiple std-daq output files.
- **Note:** Files must follow the naming format file{NN}.h5 where NN is the writer process ID.
- **Parameters:**
    - `base_path`: Directory containing the source files.
    - `output_file`: Name for the interleaved VDS file.
    - `file_prefix`: Prefix used to identify matching files.
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/h5/create_interleaved_vds" \
  -H "Content-Type: application/json" \
  -d '{"base_path": "/gpfs/test/test-beamline", "output_file": "interleaved_api_virtual.h5", "file_prefix": "file"}'
  ```

## Simulator & Test Endpoints

These endpoints are intended for testing and simulation purposes in the std-daq environment.

#### `/api/test/tomcat/server-setup`
- **Method:** POST
- **Description:** Tests the Tomcat server setup by verifying file creation and websocket connectivity.
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/test/tomcat/server-setup" -H "Content-Type: application/json" -d '{}'
  ```

### PCO Simulator Endpoints

These endpoints control the PCO camera simulator.

**IMPORTANT** `PCO` camera simulator is running in fixed `2160x2560` ROI.

#### `/api/test/tomcat/pco/simulator/start`
- **Method:** POST
- **Description:**  Starts the PCO simulator with fixed parameters such as PNG file, ZMQ endpoint, and camera address. The simulator is initiated as a background task.
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/test/tomcat/pco/simulator/start" -H "Content-Type: application/json" -d '{}'
  ```
  
#### `/api/test/tomcat/pco/simulator/stop`
- **Method:** POST
- **Description:**  Stops the running PCO simulator.
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/test/tomcat/pco/simulator/stop" -H "Content-Type: application/json" -d '{}'
  ```

### Gigafrost Simulator Endpoints

These endpoints control the Gigafrost camera simulator.

**IMPORTANT** `GigaFRoST` camera simulator is running in fixed `2016x2016` ROI.

#### `/api/test/tomcat/gigafrost/simulator/start`
- **Method:** POST
- **Description:**  Starts the Gigafrost simulator. The simulator is initiated as a background task with parameters defined by the deployment configuration.- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/test/tomcat/gigafrost/simulator/start" -H "Content-Type: application/json" -d '{}'
  ```

#### `/api/test/tomcat/gigafrost/simulator/stop`
- **Method:** POST
- **Description:**  Stops the running Gigafrost simulator.
- **Usage Example:**

  ```bash
  curl -X POST "http://xbl-daq-29:5000/api/test/tomcat/gigafrost/simulator/stop" -H "Content-Type: application/json" -d '{}'
  ```
