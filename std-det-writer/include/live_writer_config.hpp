#ifndef STD_DETECTOR_BUFFER_LIVE_WRITER_CONFIG_HPP
#define STD_DETECTOR_BUFFER_LIVE_WRITER_CONFIG_HPP

#include <cstddef>

namespace live_writer_config
{
    // N of IO threads to receive data from modules.
    const int LIVE_ZMQ_IO_THREADS = 1;

    const std::string OUTPUT_FOLDER_SYMLINK = "OUTPUT/";
}

#endif // STD_DETECTOR_BUFFER_LIVE_WRITER_CONFIG_HPP
