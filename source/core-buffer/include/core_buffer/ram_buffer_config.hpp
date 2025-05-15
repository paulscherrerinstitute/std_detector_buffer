/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

namespace cb {
struct RamBufferConfig
{
  const std::string buffer_name;
  const size_t n_bytes_data;
  const size_t n_buffer_slots;
};
} // namespace cb
