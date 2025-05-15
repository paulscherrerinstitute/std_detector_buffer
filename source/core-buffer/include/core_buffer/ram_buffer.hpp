/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include "formats.hpp"
#include "buffer_config.hpp"

class RamBuffer
{
  const std::string buffer_name_;

  const size_t n_slots_;
  const size_t data_bytes_;
  const size_t buffer_bytes_;

  int shm_fd_;
  char* buffer_;

  [[nodiscard]] static int configure_mmap_flags();
  [[nodiscard]] static size_t align_to_2mb(size_t size);
public:
  RamBuffer(std::string channel_name, size_t data_n_bytes, size_t n_slots);
  ~RamBuffer();

  void write(uint64_t id, const char* src_data);
  char* get_data(uint64_t id);
};
