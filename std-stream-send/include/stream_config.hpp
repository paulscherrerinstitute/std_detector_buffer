#ifndef STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP
#define STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP

namespace stream_config
{
    // N of IO threads to receive data from modules.
    const int STREAM_ZMQ_IO_THREADS = 1;
    // How long should the RECV queue be.
    const size_t STREAM_RCVHWM = 100;

    const int PROCESSING_ZMQ_SNDHWM = 10;
    // Keep the last second of pulses in the buffer.
    const int PULSE_ZMQ_SNDHWM = 100;

    // Number of pulses between each statistics print out.
    const size_t STREAM_STATS_MODULO = 1000;

    // reduction factor
    const int REDUCTION_FACTOR = 5;
}

#endif // STD_DETECTOR_BUFFER_STREAM_CONFIG_HPP
