/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SENDER_HPP
#define STD_DETECTOR_BUFFER_SENDER_HPP

#include "ram_buffer.hpp"
#include "send_receive_config.hpp"

namespace cb {

class Sender
{
public:
  explicit Sender(const SendReceiveConfig& config, void* zmq_context);
  void send(uint64_t id, char* meta, char* data);
  char* get_data(uint64_t id);

private:
  RamBuffer buffer;
  void* socket;
};

} // namespace cb
#endif // STD_DETECTOR_BUFFER_SENDER_HPP
