#ifndef STD_DETECTOR_BUFFER_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_RECEIVER_HPP

#include <tuple>

#include "ram_buffer.hpp"
#include "send_receive_config.hpp"

namespace cb {

class Receiver
{
public:
  explicit Receiver(const SendReceiveConfig& config, void* zmq_context);
  std::tuple<uint64_t, char*, char*> receive();

private:
  RamBuffer buffer;
  void* socket;
};

} // namespace cb
#endif // STD_DETECTOR_BUFFER_RECEIVER_HPP
