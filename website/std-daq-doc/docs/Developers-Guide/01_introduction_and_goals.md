# Introduction and Goals {#section-introduction-and-goals}

The goal of the `std-daq` is to provide an ecosystem for data acquisition from different kind of detectors with high performance limit (more than `1 GB/s`). The secondary goal is to provide generic solution within PSI ecosystem fitting purposes of different beamlines by allowing them extending functionality by extending the pipe-chain. 

## Requirements Overview {#_requirements_overview}

In case of `standard` setup the numbers should be similar to provided below - for custom setup these may differ significantly.

### **Network**

- We require about 20% overhead in the network capabilities.
- We require `melanox` connect series cards with correct configuration (datagram mode + permissions)

### **Storage**

The storage needs to be capable of ingesting single stream image (compressed or uncompressed depending on setup)

### **Server Hardware**

- We require 1 core per 1 `GB/s` of receiving/sending data
- We require 1 core per 1 Module for convertion/assembly of the image
- We require 1 core for live streaming (depending on amount of data live streamed it can be more)
- We require 1 core per 300 `MB/s` (bitshuffle) or 600 `MB/s` (blosc2) of compression
- 1 core per 3,5 `GB/s` writing data

## Quality Goals {#_quality_goals}

### Usability

Our main goal is usability - the system needs to be easy to understand and learn and attractive for the users to use. We need to focus on "first impression" so the setup of the ecosystem needs to be as user friendly as practical. This goes with documentation, installation process and updating configuration.

### Stability

The goal is to provide system that can reliably collect the needed data from detectors without degradations over time. With 99%+ uptime as a goal. It also means that updates and roll-back to working version of the system is not only feasible but also easy to execute. The stability of the system and deployment should be tested within each major release.

### Performance

The software needs to be as performant as possible without degrading the stability of itself. It should be tested not to degrade over time. Our goal is to provide a solution for commodity hardware setup for the users to reduce their costs and need of custom hardware.

## Stakeholders {#_stakeholders}

+-------------+---------------------------+---------------------------+
| Role/Name   | Contact                   | Expectations              |
+=============+===========================+===========================+
| *           | * Beamline scientists <todo name them>           | * documentation overview, debugging, understanding the extension possibilities of system, new features,        |
| End users * |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | **Missing MIA**           | * Easy deployment, setup and tuning of the system, possibility of roll-backs, debugging, monitoring, documentation        |
| Integration expert |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | * Network  <Leonardo Sala/Alvise>           | * Provide security requirements and network configuration that fulfils the `std-daq` requirements, documentation,        |
| Network admin |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | Leonardo Sala/...           | Provide standardized base image for operating system, root rights for setup/configuration phase, user access needed by end-users, toolset installation for network team,        |
| Server admin |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | Maciej/Leonardo           | development/architecture decisions       |
| Developer |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | Tadej Humar           | Overview of system and prioritization of tasks       |
| "Product Owner" |                           |                           |
+-------------+---------------------------+---------------------------+
| *           | ????           | stable interface, defined API, documentation/manual, examples,        |
| "Plug-In Developer" |                           |                           |
+-------------+---------------------------+---------------------------+
