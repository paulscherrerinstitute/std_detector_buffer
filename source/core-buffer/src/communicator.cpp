/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>

#include "communicator.hpp"
#include "buffer_utils.hpp"
#include "detectors/common.hpp"

namespace cb {

// TODO: Rewrite this into more scoped classes when communication architecture is clear.
Communicator::Communicator(const RamBufferConfig& ram_config, const CommunicatorConfig& comm_cfg)
    : buffer(ram_config.buffer_name, ram_config.n_bytes_data, ram_config.n_buffer_slots)
{
  const auto port_name = comm_cfg.stream_name;

  if (comm_cfg.connection_type == CONN_TYPE_BIND)
    socket = buffer_utils::bind_socket(comm_cfg.zmq_ctx, port_name, comm_cfg.zmq_socket_type);
  else
    socket = buffer_utils::connect_socket(comm_cfg.zmq_ctx, port_name, comm_cfg.zmq_socket_type);
}

void Communicator::send(uint64_t id, std::span<const char> meta, char* data, int flags)
{
  buffer.write(id, data);
  zmq_send(socket, meta.data(), meta.size(), flags);
}

char* Communicator::get_data(uint64_t id)
{
  return buffer.get_data(id);
}

std::tuple<uint64_t, char*> Communicator::receive(std::span<char> meta)
{
  if (auto output = zmq_recv(socket, meta.data(), meta.size(), 0); output == -1)
    return {INVALID_IMAGE_ID, nullptr};

  // First 8 bytes in any struct must represent the image_id (by convention).
  const auto id = reinterpret_cast<CommonFrame*>(meta.data())->image_id;
  return {id, buffer.get_data(id)};
}

int Communicator::receive_meta(std::span<char> meta) const
{
  return zmq_recv(socket, meta.data(), meta.size(), 0);
}

} // namespace cb
