/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "ram_buffer.hpp"
#include "ram_buffer_config.hpp"
#include "communicator_config.hpp"

#include <tuple>
#include <span>

namespace cb {

class Communicator
{
  static constexpr auto NOBLOCK = 1;
public:
  explicit Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_cfg);
  void send(uint64_t id, std::span<const char> meta, char* data, int flags = NOBLOCK);
  std::tuple<uint64_t, char*> receive(std::span<char> meta);
  int receive_meta(std::span<char> meta) const;
  char* get_data(uint64_t id);

private:
  RamBuffer buffer;
  void* socket;
};

} // namespace cb
