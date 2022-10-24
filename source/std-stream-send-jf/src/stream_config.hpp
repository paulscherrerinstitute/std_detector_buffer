/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP
#define STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP

namespace stream_config {
// N of IO threads to receive data from modules.
const int STREAM_ZMQ_IO_THREADS = 1;
// How long should the SEND queue be.
const int STREAM_ZMQ_SNDHWM = 100;
} // namespace stream_config

#endif // STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP
