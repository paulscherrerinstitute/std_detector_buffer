#include "receiver.hpp"

#include <zmq.h>

#include "buffer_utils.hpp"

namespace cb {

Receiver::Receiver(const SendReceiveConfig& config, void* zmq_context)
    : buffer(config.buffer_name,
             config.n_bytes_packet,
             config.n_bytes_data_packet * config.n_packets_frame,
             config.n_buffer_slots)
{
  socket = buffer_utils::connect_socket(zmq_context, config.buffer_name);
}

std::tuple<char*, char*> Receiver::receive()
{
  uint64_t id;
  zmq_recv(socket, &id, sizeof(id), 0);
  return {buffer.get_meta(id), buffer.get_data(id)};
}

} // namespace cb
