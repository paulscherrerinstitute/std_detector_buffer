#ifndef STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP

#include <sys/socket.h>

class PacketUdpReceiver
{

  int socket_fd_;

public:
  PacketUdpReceiver();
  virtual ~PacketUdpReceiver();

  int receive_many(mmsghdr* msgs, const size_t n_msgs);

  void bind(const uint16_t port);
  void disconnect();
};

#endif // STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP
