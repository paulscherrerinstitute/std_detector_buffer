/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>

class PacketUdpReceiver
{
  const size_t n_recv_packets_;
  const size_t n_bytes_packet_;
  int socket_fd_;

  char* packet_buffer_ = nullptr;
  iovec* recv_buff_ptr_ = nullptr;
  mmsghdr* msgs_ = nullptr;
  sockaddr_in* sock_from_ = nullptr;

  void bind(const uint16_t port);

public:
  PacketUdpReceiver(uint16_t port, size_t n_bytes_packet, size_t n_recv_packets);
  virtual ~PacketUdpReceiver();

  int receive_many();
  char* get_packet_buffer();

  void disconnect();
};

#endif // STD_DETECTOR_BUFFER_PACKET_UDP_RECEIVER_HPP
