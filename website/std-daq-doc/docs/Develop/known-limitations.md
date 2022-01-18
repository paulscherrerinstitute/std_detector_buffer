---
sidebar_position: 4
id: known-limitations
title: Known limitations
---

# Known limitations

## ZeroMQ

Zeromq version 4.1.4 (default on RH7) has a LINGER bug. Sometimes, the last 
message is not sent (the connection gets dropped before the message is in the buffer).
We are currently not using LINGER to extend our processes lifetime, so this is 
not critical. But in the future this might change, so updating to the latest 
version of ZMQ should help us prevent future bug hunting sessions.

Please install a later version:

```bash
cd /etc/yum.repos.d/
wget https://download.opensuse.org/repositories/network:messaging:zeromq:release-stable/RHEL_7/network:messaging:zeromq:release-stable.repo
yum remove zeromq
yum remove openpgm
yum install libsodium-devel
yum install zeromq-devel
```