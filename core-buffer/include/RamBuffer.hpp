#ifndef SF_DAQ_BUFFER_RAMBUFFER_HPP
#define SF_DAQ_BUFFER_RAMBUFFER_HPP

#include <string>
#include "formats.hpp"
#include "buffer_config.hpp"

class RamBuffer {
    const std::string channel_name_;
    const int n_slots_;

    const size_t meta_bytes_;
    const size_t data_bytes_;
    const size_t slot_bytes_;
    const size_t buffer_bytes_;

    int shm_fd_;
    char* buffer_;

public:
    RamBuffer(std::string channel_name, size_t meta_n_bytes, size_t data_n_bytes, int n_slots);
    ~RamBuffer();

    void write(const ModuleFrame &src_meta, const char *src_data) const;
    char* get_data(uint64_t image_id) const;
    char* get_meta(uint64_t image_id) const;


};


#endif //SF_DAQ_BUFFER_RAMBUFFER_HPP
