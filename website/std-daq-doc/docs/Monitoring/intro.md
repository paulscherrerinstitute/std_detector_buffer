---
sidebar_position: 1
id: intro
title: Introduction
---

# [std_daq_monitoring](http://daq-monitoring.psi.ch)

`std_daq_monitoring` is a monitoring service developed in parallel with `std_daq` in order to provide basic information to the developers/users about software health and status. This service is not meant to control or manage deployments and services. Official link to the website is http://daq-monitoring.psi.ch and the repository can be found [https://git.psi.ch/controls-ci/std_daq_monitoring](https://git.psi.ch/controls-ci/std_daq_monitoring).

### How the information is provided to service

`std_daq_monitoring` is using `elastic` logs provided by services through `filebeat` from interface described [here](../Interfaces/logs.md). If the logging is not sent to elastic - deployment will not be visible. **WARNING**: server where logs are produced needs to have correct time configured on system. The logs are processed only from last `10` seconds for "live" overview. 

### Information available

#### Overview

Main page shows minimal information for each available online system running and providing logs.

#### Online systems

e.g. http://daq-monitoring.psi.ch/services?deployment=gf-teststand provides information about full system - which services are online and how they are utilized

#### Error logs

http://daq-monitoring.psi.ch/errors provides overview of error log for all server available for last 1 hour by default - user can modify the time range and filter by server name.


#### Event logs

e.g. http://daq-monitoring.psi.ch/event_log?host=xbl-daq-29.psi.ch provides `[event]` logs from given server. **WARNING** if deployment is done on multiple servers only `std_det_driver` sends `event` logs from the services as it communicates directly with user. The logs are shown only from a couple of hours back. For detailed history checks one can connect directly to `elasticsearch` and process the logs.

### Development/New Features

New features can be requested via jira/issues in the repository as well as contributions are welcome (as long as they are in line with direction the tool is developed). Bug reports will be prioritized. Custom more advanced functionalities/statistics that outside the scope of this project can be developed by users directly against `elasticsearch` queries without need to modify monitoring service.
