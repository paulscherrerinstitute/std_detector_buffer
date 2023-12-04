/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <netinet/in.h>
#include "gtest/gtest.h"
#include "mock/udp.hpp"
#include "packet_udp_receiver.hpp"

#include <thread>
#include <chrono>

#include "detectors/jungfrau.hpp"

using namespace std;

TEST(PacketUdpReceiver, receive_many)
{
  uint16_t udp_port = MOCK_UDP_PORT;

  auto send_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_TRUE(send_socket_fd >= 0);

  PacketUdpReceiver udp_receiver(udp_port, sizeof(JFUdpPacket), 2);
  JFUdpPacket send_udp_buffer = {};

  auto server_address = get_server_address(udp_port);

  send_udp_buffer.bunchid = 0;
  ::sendto(send_socket_fd, &send_udp_buffer, BYTES_PER_PACKET, 0, (sockaddr*)&server_address,
           sizeof(server_address));

  send_udp_buffer.bunchid = 1;
  ::sendto(send_socket_fd, &send_udp_buffer, BYTES_PER_PACKET, 0, (sockaddr*)&server_address,
           sizeof(server_address));

  this_thread::sleep_for(chrono::milliseconds(10));

  auto n_msgs = udp_receiver.receive_many();
  ASSERT_EQ(n_msgs, 2);

  const JFUdpPacket* const packet_buffer =
      reinterpret_cast<JFUdpPacket*>(udp_receiver.get_packet_buffer());

  for (int i = 0; i < n_msgs; i++) {
    ASSERT_EQ(packet_buffer[i].bunchid, i);
  }

  n_msgs = udp_receiver.receive_many();
  ASSERT_EQ(n_msgs, -1);

  udp_receiver.disconnect();
  ::close(send_socket_fd);
}
