#include "sender.hpp"

#include <zmq.h>

#include "buffer_utils.hpp"

namespace cb {

Sender::Sender(const SendReceiveConfig& config, void* zmq_context)
    : buffer(config.buffer_name,
             config.n_bytes_packet,
             config.n_bytes_data_packet * config.n_packets_frame,
             config.n_buffer_slots)
{
  socket = buffer_utils::bind_socket(zmq_context, config.buffer_name);
}

void Sender::send(uint64_t id, char* meta, char* data)
{
  buffer.write(id, meta, data);
  zmq_send(socket, &id, sizeof(id), 0);
}

} // namespace cb
