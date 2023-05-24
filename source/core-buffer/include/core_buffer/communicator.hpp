/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMMUNICATOR_HPP
#define STD_DETECTOR_BUFFER_COMMUNICATOR_HPP

#include "ram_buffer.hpp"
#include "ram_buffer_config.hpp"
#include "communicator_config.hpp"
#include <tuple>
#include <span>

namespace cb {

class Communicator
{
public:
  explicit Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_config);
  void send(uint64_t id, std::span<const char> meta, char* data);
  std::tuple<uint64_t, char*> receive(std::span<char> meta);
  int receive_meta(std::span<char> meta);
  char* get_data(uint64_t id);
  void await_sub_connection();

private:
  RamBuffer buffer;
  void* socket;
};

} // namespace cb
#endif // STD_DETECTOR_BUFFER_COMMUNICATOR_HPP
