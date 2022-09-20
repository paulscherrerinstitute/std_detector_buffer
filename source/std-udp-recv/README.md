# std-udp-recv
std-udp-recv is the component that receives the detector data in form of UDP 
packages and writes them into a ring buffer in ram. It also sends the currently
received image_id via a ZMQ stream.

Each std-udp-recv process is taking care of a single detector module. The 
processes are all independent and do not rely on any external data input 
to maximize isolation and minimize possible interactions in our system.

We are optimizing for maintainability and long term stability. Performance is 
of concern only if the performance criteria are not met.

## Overview

std-udp-recv is a single threaded process (without counting the ZMQ IO threads)
that receives UDP packages, it assembles them into a frame and stores the metadata and 
data into ram.

### UDP receiving

Each process listens to one udp port. Packets coming to this udp port are 
assembled into frames. Frames (either complete or with missing packets) are 
passed forward. The number of received packets is saved so we can later 
(at image assembly time) determine if the frame is valid or not. At this point 
we do no validation.

We are currently using **recvmmsg** to minimize the number of switches to 
kernel mode.

We expect all packets to come in order or not come at all. Once we see the 
package for the next image_id we can assume no more packages are coming for 
the previous one, and send the assembled frame down the program.

### Writing to RAM

Frames are written one after another to a specific offset in ram. The 
offset is calculated based on the image_id, so each frame has a specific place 
and there is no need to have an index of images.

### ZMQ sending

The image_id of the assembled frame is sent via ZMQ socket. This 
is used later in the frame synchronization phase. Each module image_id is 
sent separately and the std-udp-sync unifies the individual frames into 
images.

We use the PUB/SUB mechanism for distributing the image_id - we cannot control the 
rate of the producer, and we would like to avoid distributed udp synchronization 
if possible, so PUSH/PULL does not make sense in this case.
