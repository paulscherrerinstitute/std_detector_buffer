#ifndef BUFFERCONFIG_HPP
#define BUFFERCONFIG_HPP

#include <cstddef>
#include <string>

namespace core_buffer {

    const size_t MODULE_X_SIZE = 1024;
    const size_t MODULE_Y_SIZE = 512;
    const size_t MODULE_N_PIXELS = MODULE_X_SIZE * MODULE_Y_SIZE;
    const size_t MODULE_N_BYTES = MODULE_N_PIXELS * 2;

    // How many frames we store in each file.
    // Must be power of 10 and <= than FOLDER_MOD
    const size_t FILE_MOD = 1000;

    // How many frames go into each files folder.
    // Must be power of 10 and >= than FILE_MOD.
    const size_t FOLDER_MOD = 100000;

    // Extension of our file format.
    const std::string FILE_EXTENSION = ".h5";

    // How many frames do we read at once during replay.
    const size_t REPLAY_BLOCK_SIZE = 100;

    // Size of sf_buffer RB in elements.
    const size_t BUFFER_RB_SIZE = 1000;
}

#endif //BUFFERCONFIG_HPP
