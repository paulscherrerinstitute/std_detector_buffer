/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "ram_buffer.hpp"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <chrono>
#include <iostream>

#include <date/date.h>

using namespace std;
using namespace buffer_config;
using namespace chrono;

RamBuffer::RamBuffer(string channel_name,
                     const size_t data_n_bytes,
                     const size_t n_slots)
    : buffer_name_(std::move(channel_name))
    , n_slots_(n_slots)
    , data_bytes_(data_n_bytes)
    , buffer_bytes_(data_bytes_ * n_slots_)
{
#ifdef DEBUG_OUTPUT
  using namespace date;
  cout << "[" << system_clock::now() << "]";
  cout << " [RamBuffer::RamBuffer] ";
  cout << " buffer_name " << buffer_name_;
  cout << " || n_slots " << n_slots_;
  cout << " || data_bytes " << data_bytes_;
  cout << endl;
#endif

  shm_fd_ = shm_open(buffer_name_.c_str(), O_RDWR | O_CREAT, 0777);
  if (shm_fd_ < 0) {
    throw runtime_error(string("shm_open failed: ") + strerror(errno));
  }

  if ((ftruncate(shm_fd_, static_cast<off_t>(buffer_bytes_))) == -1) {
    throw runtime_error(strerror(errno));
  }

  // TODO: Test with MAP_HUGETLB
  buffer_ = static_cast<char*>(mmap(nullptr, buffer_bytes_, PROT_WRITE, MAP_SHARED, shm_fd_, 0));
  if (buffer_ == MAP_FAILED) {
    throw runtime_error(strerror(errno));
  }
}

RamBuffer::~RamBuffer()
{
  munmap(buffer_, buffer_bytes_);
  close(shm_fd_);
  shm_unlink(buffer_name_.c_str());
}

void RamBuffer::write(const uint64_t id, const char* src_data)
{
  auto* dst_data = get_data(id);

#ifdef DEBUG_OUTPUT
  using namespace date;
  cout << "[" << system_clock::now() << "]";
  cout << " [RamBuffer::write] id " << id;
  cout << endl;
#endif

  if(src_data != nullptr)
    memcpy(dst_data, src_data, data_bytes_);
}

char* RamBuffer::get_data(const uint64_t id)
{
  const size_t slot_id = id % n_slots_;
  return buffer_ + (slot_id * data_bytes_);
}
