---
sidebar_position: 3
id: server-configuration
title: Server Configuration
---

# Server Configuration

This document outlines the essential server configuration requirements and recommendations to ensure the efficient
operation of the std_daq system, particularly in high-demand scenarios. Properly configuring your server will enhance
data acquisition performance, reduce bottlenecks, and maintain system stability.

1. UDP Receiving Buffers
   - Purpose: To ensure sufficient buffer space for incoming UDP data.
   - Configuration: Increase the size of the UDP receive buffers to handle high-throughput data:
      ```shell
  sysctl -w net.core.rmem_max=16777216
  sysctl -w net.core.rmem_default=8388608
      ```
   - Reason: Larger buffers prevent packet drops during high-throughput bursts.

2. Huge Pages for Shared Memory

   - Purpose: Use huge pages for efficient shared memory management across processes.
   - Configuration: Enable huge pages by adding the following to your boot parameters (/etc/default/grub):
   - Reason: Huge pages reduce TLB (Translation Lookaside Buffer) misses and enhance memory access speed, especially for large shared memory operations.

3. `/dev/shm` and `tmpfs` Configuration

  - Purpose: Ensure /dev/shm is properly configured to support delayed buffering and huge pages.
  - Configuration: Verify `/dev/shm` is mounted with tmpfs:
  - Reason: Proper tmpfs configuration ensures sufficient RAM allocation, alignment with huge pages, and optimal NUMA locality for memory-intensive operations.

4. Disabling IRQ Balancer

  - Purpose: Prevent the IRQ balancer from overriding custom interrupt configurations.
  - Configuration: disable IRQ balancing:
      ```shell
systemctl stop irqbalance
systemctl disable irqbalance
      ```
  - Reason: Disabling the IRQ balancer allows for manual optimization of interrupt handling to avoid conflicts.

5. Assigning Interrupts to Separate NUMA Nodes

   - Purpose: Distribute CPU load effectively by separating network interrupts from data processing tasks.
   - Configuration:
   - Reason: Ensures network packet handling is isolated from data processing, reducing contention and improving throughput.
