---
sidebar_position: 3
id: server-configuration
title: Server Configuration
---

# Server Configuration

There is a set of settings encouraged/necessary to ensure proper functionality of the `std_daq` for most demanding scenarios.

- resizing receiving buffers for udp
- enabling and configuring enough huge pages
- /dev/shm/ tmpfs configuration for delayed buffering
- disabling irq balancer
- assigning interrupts to separate numa from receivers
- 
