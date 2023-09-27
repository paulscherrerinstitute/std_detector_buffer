# Introduction and Goals {#section-introduction-and-goals}

The primary goal of `std-daq` is to offer a high-performance data acquisition ecosystem for various detectors, aiming for speeds exceeding `1 GB/s`. Additionally, we aim to provide a versatile solution within the PSI ecosystem to cater to different beamlines needs. This flexibility allows users to enhance functionality by extending the pipe-chain.

## Requirements Overview {#_requirements_overview}

For a `standard` setup, the specifications are as mentioned below. However, custom setups might vary.

### Network

- An overhead of approximately 20% in network capabilities is required.
- `melanox` connect series cards are essential, with the correct configuration (datagram mode + permissions).

### Storage

The storage needs to be capable of ingesting single stream image (compressed or uncompressed depending on setup)

### Server Hardware

- 1 core for every 1 `GB/s` of data transfer (both receiving and sending).
- 1 core for every module used in image conversion/assembly.
- 1 core dedicated to live streaming (this might increase based on the volume of live-streamed data).
- 1 core for every 300 `MB/s` (using bitshuffle) or 600 `MB/s` (using blosc2) for compression.
- 1 core for every 3.5 `GB/s` of data writing.

## Quality Goals {#_quality_goals}

### Usability

The primary focus is on usability. The system should be intuitive and easy to understand for users. The key is the "first impression", ensuring the ecosystem's setup is user-friendly. This includes aspects like documentation, the installation process, and configuration updates.

### Stability

The system should consistently and reliably collect data from detectors, aiming for an uptime of 99% or more. Updates should be straightforward, with an easy rollback option to a stable version if needed. System stability and deployment must be tested with every major release.

### Performance

The software should be optimized for performance without compromising its stability. It should not degrade over time. The objective is to offer a solution suitable for standard hardware, reducing the need for specialized equipment and associated costs.

## Stakeholders {#_stakeholders}

| Role/Name           | Contact                              | Expectations                                                                                       |
|---------------------|--------------------------------------|----------------------------------------------------------------------------------------------------|
| End users           | Beamline scientists <todo name them> | Documentation overview, debugging, understanding system extensions, requesting new features.       |
| Integration expert  | **Missing MIA**                      | Easy system deployment, setup, tuning, roll-backs, debugging, monitoring, documentation.           |
| Network admin       | Network <Leonardo Sala/Alvise>       | Security requirements, network configuration in line with `std-daq` requirements, documentation.   |
| Server admin        | Leonardo Sala/...                    | Standardized OS base image, root rights for setup/configuration, user access, toolset for network. |
| Developer           | Maciej/Leonardo                      | Development and architecture decisions.                                                            |
| "Product Owner"     | Tadej Humar                          | System overview and task prioritization.                                                           |
| "Plug-In Developer" | ????                                 | Stable interface, defined API, documentation/manual, examples.                                     |
