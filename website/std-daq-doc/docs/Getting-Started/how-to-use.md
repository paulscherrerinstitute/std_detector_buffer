---
sidebar_position: 1
id: how-to-use
title: How to Use
---

# How to use

The user's communication and interactions with the STD DAQ are performed using a combination of a REST API, which receives the incoming user's requests, and a RabbitMQ broker, which routes the incoming user's messages through a system of exchanges/queues/tag and ZMQ for the data stream.

## User interaction flow

![User interaction flow](https://github.com/paulscherrerinstitute/std_daq_service/blob/master/docs/user_interaction_flow.jpg?raw=true)

## SLS Rest interface

The currently supported basic operations are described below with usage exmaples:

### /write_sync [POST]

Triggers a synchronous write command to the writer agent.

```python
data = {"sources":"eiger", "n_images":10, "output_file":"/tmp/test.h5"}
headers = {'Content-type': 'application/json'}
r = requests.post(url = "http://<broker_address>:<port>/write_sync", json=data, headers=headers)
```

```bash
curl -X POST http://<broker_address>:<port>/write_sync -H "Content-Type: application/json" -d '{"n_images":5,"output_file":"/tmp/test.h5", "sources":"eiger"}'
```

Rest service answer:

```bash
{"request_id":"dd33c54f-2d61-4da4-96fd-c4012d75c797","response":{"end_timestamp":1627898407.391828,"init_timestamp":1627898398.259257,"output_file":"/tmp/test.h5","status":"request_success"}}
```

### /write_async [POST]

Triggers an asynchronous write command to the writer agent.

```python
data = {"sources":"eiger", "n_images":10, "output_file":"/tmp/test.h5"}
headers = {'Content-type': 'application/json'}
r = requests.post(url = "http://<broker_address>:<port>/write_async", json=data, headers=headers)
```

```bash
curl -X POST http://<broker_address>:<port>/write_async -H "Content-Type: application/json" -d '{"n_images":5,"output_file":"/tmp/test.h5", "sources":"eiger"}'
```

Rest service answer:

```bash
{"request_id":"5b50509f-e441-4f9a-8031-de10eceab9d6"}
```

### /write_kill [POST]

Kills an ongoing acquisition based on its ```request_id```.

```python
# triggers an async acquisition
data = {"sources":"eiger", "n_images":10, "output_file":"/tmp/test.h5"}
headers = {'Content-type': 'application/json'}
r = requests.post(url = "http://<broker_address>:<port>/write_async", json=data, headers=headers)
time.sleep(3)
# stores the request_id from the write_async post and kills it
req_id = str(r.json()["request_id"])
data = {"request_id":req_id}
headers = {'Content-type': 'application/json'}
r = requests.post(url = "http://<broker_address>:<port>/write_kill", json=data, headers=headers)
```

Rest service answer:

```bash
{"request_id":"88fb758a-827b-4533-bbdc-d4b68c4f410a","response":{"end_timestamp":1627908976.427919,"init_timestamp":1627908973.337146,"output_file":"/tmp/test.h5","status":"request_success"}}
```

### /detector/<det_name> [POST]

#### Eiger configuration set (POST)

```python
data = {"det_name":"eiger","config":{"frames":5,"dr":32}}
headers = {'Content-type': 'application/json'}
r = requests.post(url = "http://<broker_address>:<port>/detector", json=data, headers=headers)
```

Rest service answer:

```bash
{'response': 'request_success'}
```

:::note

When a parameter is not recognized the expected response is: 

```bash
{'response': 'Parameter not valid: <NAME_OF_NOT_VALID_PARAM>'}
```
:::


:::important

The STD DAQ rest server uses the [slsdet package](https://anaconda.org/slsdetectorgroup/slsdet) to set and get configurations to the detector. The currently covered subset of parameters are: 
- triggers
- timing
- frames
- period
- exptime
- dr
- speed
- tengiga
- threshold
:::

### /detector/<det_name> [GET]

#### Eiger configuration get (GET)
Gets the eiger detector configuration

```python
r = requests.get(url = "http://<broker_address>:<port>/detector/eiger")
```

Rest service answer:

```bash
{'det_name': 'EIGER', 'dr': 16, 'exptime': 1.0, 'frames': 100, 'period': 0.01, 'speed': 'speedLevel.FULL_SPEED', 'tengiga': True, 'threshold': -1, 'timing': 'timingMode.AUTO_TIMING', 'triggers': 1}
```

## SF Rest interface

//TODO