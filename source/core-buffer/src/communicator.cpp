/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>

#include "communicator.hpp"
#include "buffer_utils.hpp"

namespace cb {

//TODO: Rewrite this into more scoped classes when communication architecture is clear.
Communicator::Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_config)
    : buffer(ram_config.buffer_name,
             ram_config.n_bytes_meta,
             ram_config.n_bytes_data,
             ram_config.n_buffer_slots)
{
  auto port_name = ram_config.buffer_name;

  if (comm_config.connection_type == CONN_TYPE_BIND) {
    socket = buffer_utils::bind_socket(comm_config.zmq_ctx, port_name, comm_config.zmq_socket_type);
  }
  else {
    socket =
        buffer_utils::connect_socket(comm_config.zmq_ctx, port_name, comm_config.zmq_socket_type);
  }
}

void Communicator::send(uint64_t id, char* meta, char* data)
{
  buffer.write(id, meta, data);
  zmq_send(socket, &id, sizeof(id), 0);
}

char* Communicator::get_data(uint64_t id)
{
  return buffer.get_data(id);
}

std::tuple<uint64_t, char*, char*> Communicator::receive()
{
  uint64_t id;
  zmq_recv(socket, &id, sizeof(id), 0);
  return {id, buffer.get_meta(id), buffer.get_data(id)};
}

} // namespace cb