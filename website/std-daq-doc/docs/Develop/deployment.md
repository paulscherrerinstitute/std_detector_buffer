---
sidebar_position: 2
id: deployment
title: Deployment
---

# Deployment

The deployment of the STD-DAQ is performed using [Ansible](https://www.ansible.com/) and it can be triggered from the internal psi's gitlab server (permissions apply). 

## Trigger

The pipeline are organized according to each beamline and are executed via gitlab-runner. Check [std_daq_configs](https://git.psi.ch/std_daq_configs/)


### Beamline configuration file

The ansible uses a beamline definitions that coordinates which services should be deployed on each node. Check [beamlines.yaml](https://git.psi.ch/hpdi_configs/sls/-/blob/master/beamlines.yaml)

## Services/roles

The services/roles are defined and fetched from [roles](https://git.psi.ch/HPDI/daq_server_deployment/-/tree/master/roles)

### Configuration file

Certain services/roles make use of a json configuration file which exists in the [hosts](https://git.psi.ch/hpdi_configs/sls/-/tree/master/hosts/).