#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <fmt/core.h>
#include <thread>
#include <vector>
#include "udp_recv_config.hpp"


const int UDP_BUFFER_MAX_SIZE = 1024 * 10;
const int BUFFER_UDP_US_TIMEOUT = 1000 * 100;
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

  char* buffer = new char[UDP_BUFFER_MAX_SIZE];

  while (run) {

  }
  delete[] buffer;
}


int main(int argc, char** argv)
{

  if (argc != 2) {
    cout << endl;
    cout << "Usage: std_udp_dump [detector_json_filename]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << endl;
    exit(-1);
  }

  const auto config = UdpRecvConfig::from_json_file(string(argv[1]));

  std::vector<std::thread> threads;
  atomic_bool running(true);

  for (int i=0; i<config.n_modules; i++) {
    auto udp_port = config.start_udp_port + i;
    threads.emplace_back(receive_and_dump, udp_port, std::ref(running));
  }

  cin.get();
  running = false;

  for (auto& thread : threads) {
    thread.join();
  }
}
