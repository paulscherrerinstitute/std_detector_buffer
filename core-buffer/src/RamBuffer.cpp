#include <sys/mman.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include "RamBuffer.hpp"
#include "date.h"
#include <chrono>
#include <iostream>


using namespace std;
using namespace buffer_config;

RamBuffer::RamBuffer(string channel_name,
                     const size_t meta_n_bytes,
                     const size_t data_n_bytes,
                     const int n_slots) :
        channel_name_(std::move(channel_name)),
        n_slots_(n_slots),
        meta_bytes_(meta_n_bytes),
        data_bytes_(data_n_bytes),
        slot_bytes_((meta_bytes_ + data_bytes_)),
        buffer_bytes_(slot_bytes_ * n_slots_)
{
#ifdef DEBUG_OUTPUT
    using namespace date;
    cout << " [" << std::chrono::system_clock::now();
    cout << "] [RamBuffer::RamBuffer] ";
    cout << " buffer_name_ " << channel_name_;
    cout << " || n_slots_ " << n_slots_;
    cout << " || meta_bytes_" << meta_bytes_;
    cout << " || data_bytes_" << data_bytes_;
    cout << endl;
#endif

    shm_fd_ = shm_open(channel_name_.c_str(), O_RDWR | O_CREAT, 0777);
    if (shm_fd_ < 0) {
        throw runtime_error(string("shm_open failed: ") + strerror(errno));
    }

    if ((ftruncate(shm_fd_, buffer_bytes_)) == -1) {
        throw runtime_error(strerror(errno));
    }

    // TODO: Test with MAP_HUGETLB
    buffer_ = static_cast<char *>(mmap(NULL, buffer_bytes_, PROT_WRITE,
                                       MAP_SHARED, shm_fd_, 0));
    if (buffer_ == MAP_FAILED) {
        throw runtime_error(strerror(errno));
    }
}

RamBuffer::~RamBuffer()
{
    munmap(buffer_, buffer_bytes_);
    close(shm_fd_);
    shm_unlink(channel_name_.c_str());
}

void RamBuffer::write(const ModuleFrame& src_meta, const char *src_data) const
{
    auto *dst_meta = (ModuleFrame*) get_meta( src_meta.id);
    auto *dst_data = get_data( src_meta.id);

    #ifdef DEBUG_OUTPUT
        using namespace date;
        cout << " [" << std::chrono::system_clock::now();
        cout << "] [RamBuffer::write_frame] ";
        cout << " src_meta.id " << src_meta.id;
        cout << " || src_meta.frame_index " << src_meta.frame_index;
        cout << " || src_meta.n_recv_packets " << src_meta.n_recv_packets;
        cout << " || src_meta.daq_rec " << src_meta.daq_rec;
        cout << " || src_meta.module_id " << src_meta.module_id;
        cout << endl;
    #endif

    memcpy(dst_meta, &src_meta, sizeof(ModuleFrame));
    memcpy(dst_data, src_data, data_bytes_);
}

char* RamBuffer::get_meta(const uint64_t image_id) const
{
    const size_t slot_id = image_id % n_slots_;
    return buffer_ + (slot_id * slot_bytes_);
}

char* RamBuffer::get_data(const uint64_t image_id) const
{
    const size_t slot_id = image_id % n_slots_;
    return buffer_ + (slot_id * slot_bytes_) + meta_bytes_;
}
