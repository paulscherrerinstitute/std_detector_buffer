#include <netinet/in.h>
#include <iostream>
#include <stdexcept>
#include "PacketUdpReceiver.hpp"
#include "jungfrau.hpp"
#include <unistd.h>
#include <cstring>
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

PacketUdpReceiver::PacketUdpReceiver(
    const uint16_t port, const size_t n_bytes_packet, const size_t n_recv_packets) :
    n_recv_packets_(n_recv_packets),
    n_bytes_packet_(n_bytes_packet),
    socket_fd_(-1)
{
  bind(port);

//TODO: Posix align this memory.
  packet_buffer_ = new char[n_recv_packets_ * n_bytes_packet_];
  recv_buff_ptr_ = new iovec[n_recv_packets_];
  msgs_ = new mmsghdr[n_recv_packets_];
  sock_from_ = new sockaddr_in[n_recv_packets_];

  for (size_t i = 0; i < n_recv_packets_; i++) {
    recv_buff_ptr_[i].iov_base = (void*) &(packet_buffer_[i * n_bytes_packet_]);
    recv_buff_ptr_[i].iov_len = n_bytes_packet_;

    msgs_[i].msg_hdr.msg_iov = &recv_buff_ptr_[i];
    msgs_[i].msg_hdr.msg_iovlen = 1;
    msgs_[i].msg_hdr.msg_name = &sock_from_[i];
    msgs_[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
  }
}

PacketUdpReceiver::~PacketUdpReceiver()
{
  disconnect();
}

void PacketUdpReceiver::bind(const uint16_t port)
{
  if (socket_fd_ > -1) {
    throw runtime_error("Socket already bound.");
  }

  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    throw runtime_error("Cannot open socket.");
  }

  sockaddr_in server_address = {};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);

  timeval udp_socket_timeout;
  udp_socket_timeout.tv_sec = 0;
  udp_socket_timeout.tv_usec = BUFFER_UDP_US_TIMEOUT;

  if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &udp_socket_timeout, sizeof(timeval)) == -1) {
    throw runtime_error("Cannot set SO_RCVTIMEO. " + string(strerror(errno)));
  }

  if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &BUFFER_UDP_RCVBUF_BYTES, sizeof(int)) == -1) {
    throw runtime_error("Cannot set SO_RCVBUF. " + string(strerror(errno)));
  };

  // TODO: try to set SO_RCVLOWAT
  auto bind_result = ::bind(socket_fd_, reinterpret_cast<const sockaddr*>(&server_address),
                            sizeof(server_address));

  if (bind_result < 0) {
    throw runtime_error("Cannot bind socket.");
  }
}

char* PacketUdpReceiver::get_packet_buffer()
{
  return packet_buffer_;
}

int PacketUdpReceiver::receive_many()
{
  return recvmmsg(socket_fd_, msgs_, n_recv_packets_, 0, 0);
}

void PacketUdpReceiver::disconnect()
{
  close(socket_fd_);
  socket_fd_ = -1;

  delete[] packet_buffer_;
  delete[] recv_buff_ptr_;
  delete[] msgs_;
  delete[] sock_from_;
}
