/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "receiver.hpp"

#include <zmq.h>

#include "buffer_utils.hpp"

namespace cb {

Receiver::Receiver(const SendReceiveConfig& config, void* zmq_context)
    : buffer(config.buffer_name, config.n_bytes_meta, config.n_bytes_data, config.n_buffer_slots)
{
  auto port_name = config.buffer_name;
  socket = buffer_utils::connect_socket(zmq_context, port_name);
}

std::tuple<uint64_t, char*, char*> Receiver::receive()
{
  uint64_t id;
  zmq_recv(socket, &id, sizeof(id), 0);
  return {id, buffer.get_meta(id), buffer.get_data(id)};
}

} // namespace cb
