#ifndef STD_DETECTOR_BUFFER_RAM_BUFFER_HPP
#define STD_DETECTOR_BUFFER_RAM_BUFFER_HPP

#include <string>
#include "formats.hpp"
#include "buffer_config.hpp"

class RamBuffer
{
  const std::string buffer_name_;

  const size_t n_slots_;
  const size_t meta_bytes_;
  const size_t data_bytes_;
  const size_t slot_bytes_;
  const size_t buffer_bytes_;

  int shm_fd_;
  char* buffer_;

public:
  RamBuffer(std::string channel_name, size_t meta_n_bytes, size_t data_n_bytes, size_t n_slots);
  ~RamBuffer();

  void write(uint64_t id, const char* src_meta, const char* src_data);
  char* get_data(uint64_t id);
  char* get_meta(uint64_t id);
};

#endif // STD_DETECTOR_BUFFER_RAM_BUFFER_HPP
