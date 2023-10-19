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
| **REST API**                     | TBD                                                                                                                                                                                                                                                                       |
## Details {#_details}
### **Live image data and metadata**

This section describes the common interface used for communication between services within the system. The design allows for the addition of new processing services by beamline users in the future. To integrate with the existing pipeline, these services must follow the current communication protocol to send and receive data to other standard services. The protocol is split into two channels: metadata and image data, allowing services to interact with metadata, avoiding unnecessary transfer or reading of image data if not needed.

1. **Metadata Transmission**:
    - Metadata for images is transmitted via a `ZeroMQ` Publish/Subscribe (pub/sub) connection using Inter-Process Communication (IPC). The messages follow a standard `Protobuf` structure, detailed [HERE](#protobuf-documentation).

2. **Image Data Storage**:
    - Image data is stored in shared memory within a RAM buffer, utilizing a ring buffer mechanism to hold a fixed number of images. Each raw image data is accessible via an `image_id`.

### **Live Streaming Data**
### **BSREAD PCO Camera Input**
### **Stored Image Files**
### **Standard DAQ Config File**
### **Ansible Deployment File**
### **Logging Protocol**
### **REST API**
