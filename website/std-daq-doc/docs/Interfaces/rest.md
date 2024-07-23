---
sidebar_position: 9
id: rest
title: Config Rest Interface
---

## Config Rest Interface

The Config Rest Interface, implemented in Python, allows the users of std-daq to get and set the configuration file that parametrizes std-daq microservices. The configuration file is stored locally on the servers in which the std-daq runs and, after each change, a restart of the std-daq microservices is automatically done in order to adopt the new configuration (the restart might take a couple of seconds until the std-daq is up and running again). 

The configuration file is defined per deployment and the user can only change the contents in it. That means that the user can't change which file is being used for the configuration of the std-daq deployment - that needs to be done on the ansible deployment playbook by the integrator. 

**Please, note that a misconfigured camera or detector configuration file can result in malfunctioning of the std-daq.** 

The config files are stored on the [config repo](https://git.psi.ch/std_daq_configs) and, when a deployment is done, such repo is cloned onto the servers (located in the folder ```/etc/std_daq/configs/```) and used for the parametrization of the std-daq services. 

Please, take into considerations that local changes done via the RestAPI are not going to be permanent. In case of a std-daq redeployment the file is going to be overwritten and changes are going to be lost. If you need a persistent change in the config file, make sure to submit a merge request via the [config repo](https://git.psi.ch/std_daq_configs)

### Implementation details

The Config Rest Interface is a dockerized application that is configured using ansible playbooks, for example [this one](https://git.psi.ch/std_daq_configs/sls/poc_fast_deploy/-/blob/master/tomcat.gf1.h5_compression.yml?ref_type=heads)and uses  [this role](https://git.psi.ch/controls-ci/psi.std_daq_rest). 

The Config Rest Interface can configured using one or two servers. In the case of two servers, the changes performed via the primary rest interface are going to be submitted and synchronized to the secondary server. This is the case for the GigaFRoST deployment, in which the std-daq is deployed in two different servers, namely the backend server and the writer server. 

Note that the configuration file needs to be identical on both, in order that the std-daq is running on both servers with the same identical parameters. Therefore, the synchronization between the config files is necessary, Config Rest Interface offers this feature and it be configured in the ansible playook.

### RestAPI endpoints

#### Config endpoints

##### /api/config/get

The `get` endpoint returns the content of the configuration file, defined at the deployment of the std-daq. It requires a argument parameter username, which is going to be stored in the logs for completeness.

A configuration can be, for example: 

```json
{
	"detector_name": "gf-teststand",
	"detector_type": "gigafrost",
	"n_modules": 8,
	"bit_depth": 16,
	"image_pixel_height": 128,
	"image_pixel_width": 480,
	"start_udp_port": 2000,
	"submodule_info": {},
	"max_number_of_forwarders_spawned": 10,
	"sender_sends_full_images": true,
	"use_all_forwarders": true,
	"module_sync_queue_size": 4096,
	"number_of_writers": 14,
	"module_positions": {}
}
```

###### Usage examples:

```bash
curl -X GET "http://xbl-daq-29:5000/api/config/get?user=username"
```

```python
import getpass
import json
import requests

response = requests.get('xbl-daq-29:5000/api/config/get', params={'user': getpass.getuser()})
response.raise_for_status()
config = response.json()
print(json.dumps(config, indent=4))
```

##### /api/config/set

The `set` endpoint writes a **fully composed** configuration into the configuration file will be used in the std-daq, once validated and saved, an automatic restart on the std-daq services will be done and the new configuration is going to be adopted. 

**Note: the configuration must contain a fully valid configuration. Missing fields and partially valid configurations are going to be rejected.**



###### Json Config Schema

Every submitted configuration, before being saved, will be validated against a defined schema, see below:

```json
{
	"detector_name": {"type": "string"},
	"detector_type": {"type": "string"},
	"n_modules": {"type": "integer"},
	"bit_depth": {"type": "integer"},
	"image_pixel_height": {"type": "integer"},
	"image_pixel_width": {"type": "integer"},
	"start_udp_port": {"type": "integer"},
	"writer_user_id": {"type": "integer"},
	"submodule_info": {"type": "object"},
	"max_number_of_forwarders_spawned": {"type": "integer"},
	"use_all_forwarders": {"type": "boolean"},
	"module_sync_queue_size": {"type": "integer"},
	"module_positions": {"type": "object"},
}
```

###### Usage examples:

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
payload = {'user': current_user}
response = requests.post('xbl-daq-29:5000/api/config/set', params=payload, data=json.dumps(new_config), headers=headers)
response.raise_for_status()
print(response.json())
```


#### H5 Endpoints

The ```H5 endpoints``` are intented to support the user in the management and analysis of std-daq generated files.

##### /api/h5/read_metadata

The ```read_metadata``` function takes the filename and returns the image id and status of such image in the given file.

##### Usage example

```bash
curl -G "http://xbl-daq-29:5000/api/h5/read_metadata" --data-urlencode "filename=/gpfs/test/test-beamline/interleaved_api_virtual.h5
```


#### /api/h5/get_metadata_status

This ```get_metadata_status``` function gives a peak inside the contents of the file: list of datasets, shapes, types, and names and metadata.

##### Usage example
```bash
curl -G "http://xbl-daq-29:5000/api/h5/get_metadata_status" --data-urlencode "filename=/gpfs/test/test-beamline/interleaved_api_virtual.h5
```

#### /api/h5/create_interleaved_vds

This function will combine the std-daq written files from a given folder in a interleaved h5 virtual dataset.

**Note that the name format of the files need to follow ```file{NN}.h5```, where NN represents the writer process id used when it was written.**

##### Usage example

```bash
curl -X POST "http://xbl-daq-29:5000/api/h5/create_interleaved_vds" -H "Content-Type: application/json" -d '{"base_path": "/gpfs/test/test-beamline", "output_file": "interleaved_api_virtual.h5"}'
```