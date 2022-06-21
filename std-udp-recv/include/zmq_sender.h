#ifndef STD_DETECTOR_BUFFER_ZMQ_SENDER_H
#define STD_DETECTOR_BUFFER_ZMQ_SENDER_H

#include "ram_buffer.hpp"
#include "packet_udp_receiver.hpp"

struct SenderConfig
{
  const std::string buffer_name;
  const size_t n_bytes_packet;
  const size_t n_bytes_data_packet;
  const size_t n_packets_frame;
  const size_t n_buffer_slots;
};

class ZmqSender
{
  RamBuffer buffer;
  void* socket;

public:
  ZmqSender(SenderConfig config);
  void send(uint64_t id, char* meta, char* data);
};

#endif // STD_DETECTOR_BUFFER_ZMQ_SENDER_H
