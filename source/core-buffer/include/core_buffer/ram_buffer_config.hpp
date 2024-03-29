/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_RAM_BUFFER_CONFIG_HPP
#define STD_DETECTOR_BUFFER_RAM_BUFFER_CONFIG_HPP

namespace cb {
struct RamBufferConfig
{
  const std::string buffer_name;
  const size_t n_bytes_data;
  const size_t n_buffer_slots;
};
} // namespace cb

#endif // STD_DETECTOR_BUFFER_RAM_BUFFER_CONFIG_HPP
