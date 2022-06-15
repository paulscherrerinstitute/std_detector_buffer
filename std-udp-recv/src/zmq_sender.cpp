#include <zmq.h>
#include "../include/zmq_sender.h"
#include "buffer_utils.hpp"

ZmqSender::ZmqSender(SenderConfig config)
    : buffer(config.buffer_name, config.n_bytes_packet,
             config.n_bytes_data_packet * config.n_packets_frame, config.n_buffer_slots)
{
  auto ctx = zmq_ctx_new();
  socket = buffer_utils::bind_socket(ctx, config.buffer_name);



}
void ZmqSender::send(uint64_t id, char* meta, char* data)
{
  buffer.write(id, meta, data);
  zmq_send(socket, &id, sizeof(id), 0);
}
