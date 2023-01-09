/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>

#include "communicator.hpp"
#include "buffer_utils.hpp"
#include "detectors/common.hpp"

namespace cb {

//TODO: Rewrite this into more scoped classes when communication architecture is clear.
Communicator::Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_config)
    : buffer(ram_config.buffer_name,
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

void Communicator::send(uint64_t id, std::span<const char> meta, char* data)
{
  buffer.write(id, data);
  zmq_send(socket, meta.data(), meta.size(), 0);
}

char* Communicator::get_data(uint64_t id)
{
  return buffer.get_data(id);
}

std::tuple<uint64_t, char*> Communicator::receive(std::span<char> meta)
{
  zmq_recv(socket, meta.data(), meta.size(), 0);

  // First 8 bytes in any struct must represent the image_id (by convention).
  const auto id = reinterpret_cast<CommonFrame*>(meta.data())->image_id;
  return {id, buffer.get_data(id)};
}

} // namespace cb
