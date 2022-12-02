---
sidebar_position: 2
id: faq
title: FAQ
---

# Todo

## Error msg: shm_open failed: Permission denied

### Verify the permissions of the shared memory buffer

`ls -ahl /dev/shm/`

it needs to be 777:

-rwxrwxrwx. 1 dbe dbe 1001M Mar 16 11:04 cSAXS.EG01V01
-rwxrwxrwx. 1 dbe dbe 1010M Mar 16 11:04 cSAXS.EG01V01_assembler

if it's not, one must fix it:

`chmod 777 /dev/shm/cSAXS.EG01V01*`

### Verify the permission of the zmq ipc folders

`ls -ahl /tmp/ | grep std`

it needs to be owned by dbe:

```
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 10:55 std-daq-cSAXS.EG01V01-0
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 10:55 std-daq-cSAXS.EG01V01-1
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 10:55 std-daq-cSAXS.EG01V01-2
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 10:55 std-daq-cSAXS.EG01V01-3
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 11:04 std-daq-cSAXS.EG01V01-assembler
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 11:04 std-daq-cSAXS.EG01V01-streamer
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 11:18 std-daq-cSAXS.EG01V01-sync
srwxr-xr-x.  1 dbe  dbe     0 Mar 16 11:43 std-daq-cSAXS.EG01V01-writer_agent
```

if it's not, one must fix it:

`chown -Rv dbe:dbe /tmp/std-daq*`
