
# Solution Strategy {#section-solution-strategy}
- Streaming oriented architecture to allow interactive feedback with users. 
- Microservice architecture allows us to scale with the detector modules and implement different features for different users on demand.
- Separation of data and metadata in order to allow no-copy data transfer between services and allow for metadata based image filtering (not possibe on full data due to throughput issue)
- Zmq based streaming for user interfaces to allow for client scaling independent of services and easy interoperability with existing ifrastructure
- Data pipe only - not checking data quality or correctness of sources (what comes in goes out)
- Single source of config for all services - single source of truth, common service interface
- Systemd based services management - restarting, logging, configuration, 

### **Contents**

A short summary and explanation of the fundamental decisions and
solution strategies, that shape system architecture. It includes

-   technology decisions

-   decisions about the top-level decomposition of the system, e.g.
    usage of an architectural pattern or design pattern

-   decisions on how to achieve key quality goals

-   relevant organizational decisions, e.g. selecting a development
    process or delegating certain tasks to third parties.

### **Motivation**



These decisions form the cornerstones for your architecture. They are
the foundation for many other detailed decisions or implementation

### **Form**

Keep the explanations of such key decisions short.

Motivate what was decided and why it was decided that way, based upon
problem statement, quality goals and key constraints. Refer to details
in the following sections.

See [Solution Strategy](https://docs.arc42.org/section-4/) in the arc42
documentation.




- Metadata + RamBuffer
- array-1.0
- bsread
- hdf5 files
- config file
- deployment file
- REST API
- influx DB logging

Commodity HW. low level high performance, standardized solution. cost of integration, hide complexity of getting data.
Scalable, flexible, local support/knowledge, integrated in PSI ecosystem, standardized deployment. Plug-in
functionality, Tight integration with the detectors.

- We support up to x(~20 000) receiving events per core per second.
- We support up to 1GB tput/s per core true both for receiving and sending out.

stability > setup simplicity > performance > user experience > testability > debug-ability > feature rich
