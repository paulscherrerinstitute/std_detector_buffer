# System Scope and Context {#section-system-scope-and-context}

This system is designed to receive input data from detectors or cameras, convert this data into a processable grayscale
2D image, and provide the image data to users either as a file or as a streaming service.

## External Interfaces

| Interface                        | Description                                                                                                                                                                                                                                                               |
|----------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Live image data and metadata** | Image metadata is encapsulated within a fixed interface defined by `protobuf` files. This metadata is transmitted via a `ZeroMQ` (zmq) socket using a Publish/Subscribe (pub/sub) pattern. Image data is made available through shared memory using a common ring buffer. |
| **Live Streaming Data**          | Live image data is streamed using the `array-1.0` protocol via a `ZeroMQ` (zmq) Publish/Subscribe (pub/sub) connection.                                                                                                                                                   |
| **BSREAD PCO Camera Input**      | The services accept input from cameras through four parallel `ZeroMQ` (zmq) Push/Pull connections.                                                                                                                                                                        |
| **Stored Image Files**           | Images are saved in `HDF5` file format, synchronized with PSI requirements for user access.                                                                                                                                                                               |
| **Standard DAQ Config File**     | A `JSON` configuration file, common across all services, is provided as a startup argument to configure the Data Acquisition (DAQ) system.                                                                                                                                |
| **Ansible Deployment File**      | An `Ansible` playbook is utilized for setting up and deploying all services on DAQ servers.                                                                                                                                                                               |
| **Logging Protocol**             | Logging is facilitated using the `InfluxDB` protocol with logs being forwarded by `Filebeat` to `Elasticsearch` for centralized logging and analysis.                                                                                                                     |
| **REST API**                     | <tbd>                                                                                                                                                                                                                                                                     |

### Live image data and metadata

This interface is a common interface to communicate between services...
