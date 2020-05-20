#ifndef SF_DAQ_BUFFER_REPLAYH5READER_HPP
#define SF_DAQ_BUFFER_REPLAYH5READER_HPP

#include <string>
#include "jungfrau.hpp"
#include <H5Cpp.h>
#include <memory>
#include "buffer_config.hpp"


class ReplayH5Reader {

    const std::string device_;
    const std::string channel_name_;

    H5::H5File current_file_;
    std::string current_filename_;
    H5::DataSet dset_metadata_;
    H5::DataSet dset_frame_;

    std::unique_ptr<char[]> frame_buffer_ = std::make_unique<char[]>(
        core_buffer::MODULE_N_BYTES * core_buffer::REPLAY_READ_BUFFER_SIZE);
    std::unique_ptr<ModuleFrame[]> metadata_buffer_ =
            std::make_unique<ModuleFrame[]>(core_buffer::FILE_MOD);

    uint64_t buffer_start_pulse_id_ = 0;
    uint64_t buffer_end_pulse_id_ = 0;
    void prepare_buffer_for_pulse(const uint64_t pulse_id);

public:
    ReplayH5Reader(const std::string device, const std::string channel_name);
    virtual ~ReplayH5Reader();
    void close_file();
    bool get_frame(
            const uint64_t pulse_id, ModuleFrame* metadata, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_REPLAYH5READER_HPP
