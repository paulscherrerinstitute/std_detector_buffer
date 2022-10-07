/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "sender.hpp"

#include <zmq.h>

#include "buffer_utils.hpp"

namespace cb {

Sender::Sender(const SendReceiveConfig& config, void* zmq_context)
    : buffer(config.buffer_name, config.n_bytes_meta, config.n_bytes_data, config.n_buffer_slots)
{
  auto port_name = config.buffer_name;
  socket = buffer_utils::bind_socket(zmq_context, port_name);
}

void Sender::send(uint64_t id, char* meta, char* data)
{
  buffer.write(id, meta, data);
  zmq_send(socket, &id, sizeof(id), 0);
}

char* Sender::get_data(uint64_t id)
{
  return buffer.get_data(id);
}

} // namespace cb
