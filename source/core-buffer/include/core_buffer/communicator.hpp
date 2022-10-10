/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMMUNICATOR_HPP
#define STD_DETECTOR_BUFFER_COMMUNICATOR_HPP

#include "ram_buffer.hpp"
#include "ram_buffer_config.hpp"
#include "communicator_config.hpp"
#include <tuple>

namespace cb {

class Communicator
{
public:
  explicit Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_config);
  void send(uint64_t id, char* meta, char* data);
  std::tuple<uint64_t, char*, char*> receive();
  char* get_data(uint64_t id);

private:
  RamBuffer buffer;
  void* socket;
};

} // namespace cb
#endif // STD_DETECTOR_BUFFER_COMMUNICATOR_HPP
