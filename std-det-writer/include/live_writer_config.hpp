#ifndef STD_DET_WRITER_LIVE_WRITER_CONFIG_HPP
#define STD_DET_WRITER_LIVE_WRITER_CONFIG_HPP

#include <cstddef>

namespace live_writer_config
{
    // N of IO threads to receive data from modules.
    const int LIVE_ZMQ_IO_THREADS = 1;

    const std::string OUTPUT_FOLDER_SYMLINK = "OUTPUT/";
}

#endif //STD_DET_WRITER_LIVE_WRITER_CONFIG_HPP