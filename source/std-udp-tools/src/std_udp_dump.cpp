/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

#include <fmt/core.h>

#include "udp_recv_config.hpp"


// Since jumbo frames are 9000, there should be no packet larger than that.
const int UDP_BUFFER_MAX_SIZE = 1024 * 10;
// 100ms timeout for stopping the program.
const int BUFFER_UDP_US_TIMEOUT = 1000 * 100;
// Buffer for each UDP connection - 20MB should be enough.
const int BUFFER_UDP_RCVBUF_BYTES = 1024 * 1024 * 20;

using namespace std;

int bind_udp_socket(uint16_t udp_port)
{
  auto socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socket_fd < 0) {
    throw runtime_error("Cannot open socket.");
  }

  sockaddr_in server_address = {};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(udp_port);

  timeval udp_socket_timeout;
  udp_socket_timeout.tv_sec = 0;
  udp_socket_timeout.tv_usec = BUFFER_UDP_US_TIMEOUT;

  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &udp_socket_timeout, sizeof(timeval)) == -1) {
    throw runtime_error("Cannot set SO_RCVTIMEO. " + string(strerror(errno)));
  }

  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &BUFFER_UDP_RCVBUF_BYTES, sizeof(int)) == -1) {
    throw runtime_error("Cannot set SO_RCVBUF. " + string(strerror(errno)));
  };

  auto bind_result = ::bind(socket_fd, reinterpret_cast<const sockaddr*>(&server_address),
                            sizeof(server_address));

  if (bind_result < 0) {
    throw runtime_error("Cannot bind socket.");
  }

  return socket_fd;
}

void receive_and_dump(uint16_t udp_port, atomic_bool& run)
{
  auto socket_fd = bind_udp_socket(udp_port);
  const auto file_name = fmt::format("{}.dat", udp_port);
  ofstream file (file_name, ios::out | ios::binary);

  char* buffer = new char[UDP_BUFFER_MAX_SIZE];
  size_t n_packets = 0;

  fmt::print("[{}] Receiver dumping to {}\n", udp_port, file_name);

  while (run) {
    const int n_recv_bytes = recv(socket_fd, buffer, UDP_BUFFER_MAX_SIZE, 0);
    if (n_recv_bytes < 0) {
      if (errno == EAGAIN) {
        continue;
      }
      throw std::runtime_error(fmt::format("Cannot recv from socket, error: {}", errno));
    }

    if (n_recv_bytes == UDP_BUFFER_MAX_SIZE) {
      throw std::runtime_error("Current udp buffer to small for received packet.");
    }

    n_packets += 1;
    fmt::print("[{}] Received {} packets.\n", udp_port, n_packets);

    const uint64_t udp_total_bytes = n_recv_bytes;
    file.write(reinterpret_cast<const char*>(&udp_total_bytes), sizeof(udp_total_bytes));
    file.write(buffer, n_recv_bytes);
    file.flush();
  }

  file.close();
  delete[] buffer;
}


int main(int argc, char** argv)
{

  if (argc != 2) {
    cout << endl;
    cout << "Usage: std_udp_tools_dump [detector_json_filename]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << endl;
    exit(-1);
  }

  const auto config = UdpRecvConfig::from_json_file(string(argv[1]));

  std::vector<std::thread> threads;
  atomic_bool running(true);

  fmt::print("Starting UDP dump for detector {} with n_modules {}\n",
             config.detector_name, config.n_modules);

  for (int i=0; i<config.n_modules; i++) {
    auto udp_port = config.start_udp_port + i;
    threads.emplace_back(receive_and_dump, udp_port, std::ref(running));
  }

  // Delay a bit so all threads report they started.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  fmt::print("Press enter to stop the program...\n");

  cin.get();
  running = false;

  fmt::print("Closing sockets.\n");

  for (auto& thread : threads) {
    thread.join();
  }
}
