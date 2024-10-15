/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_BUFFER_CONFIG_HPP
#define STD_DETECTOR_BUFFER_BUFFER_CONFIG_HPP

#include <cstddef>
#include <string>

namespace buffer_config {

// Size of UDP recv buffer
inline constexpr int BUFFER_UDP_RCVBUF_N_SLOTS = 100;
// 8246 bytes for each UDP packet.
inline constexpr int BUFFER_UDP_RCVBUF_BYTES = (128 * BUFFER_UDP_RCVBUF_N_SLOTS * 8246);
// Microseconds timeout for UDP recv.
inline constexpr int BUFFER_UDP_US_TIMEOUT = 2 * 1000;
// HWM for live stream from buffer.
inline constexpr int BUFFER_ZMQ_SNDHWM = 50000;
// HWM for live stream from buffer.
inline constexpr int BUFFER_ZMQ_RCVHWM = 50000;
// IPC address of the live stream.
inline constexpr std::string IPC_URL_BASE = "ipc:///tmp/";
// Number of image slots in ram buffer - 10 seconds should be enough
inline constexpr int RAM_BUFFER_N_SLOTS = 10 * 100;
// Number of image slots in ram buffer for receivers - this can be fixed
// as this is a reasonable minimal amount that is required for receivers to correctly function
inline constexpr int RECEIVER_RAM_BUFFER_N_SLOTS = 10 * 100;
} // namespace buffer_config

#endif // STD_DETECTOR_BUFFER_BUFFER_CONFIG_HPP
