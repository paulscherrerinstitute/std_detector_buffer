/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <netinet/in.h>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <fstream>

#include <fmt/core.h>

#include "utils/utils.hpp"

const int UDP_BUFFER_MAX_SIZE = 1024 * 10;
const int BUFFER_UDP_US_TIMEOUT = 1000 * 100;
const int BUFFER_UDP_RCVBUF_BYTES = 1024 * 1024 * 20;
const int BARRIER_PACKET_COUNT = 100;

using namespace std;
std::mutex barrier_mutex;

int connect_udp_socket(uint16_t udp_port)
{
  auto socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socket_fd < 0) {
    throw runtime_error("Cannot open socket.");
  }

  sockaddr_in server_address = {};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(udp_port);

  auto connect_result = ::connect(socket_fd, reinterpret_cast<const sockaddr*>(&server_address),
                                  sizeof(server_address));

  if (connect_result < 0) {
    throw runtime_error("Cannot connect socket.");
  }

  return socket_fd;
}

void read_and_send(const uint16_t udp_port,
                   atomic_bool& run,
                   const int ms_delay,
                   const string path_folder)
{
  auto socket_fd = connect_udp_socket(udp_port);
  const auto file_name = fmt::format("{}/{}.dat", path_folder, udp_port);

  char* buffer = new char[UDP_BUFFER_MAX_SIZE];
  size_t n_packets = 0;
  // Read from file
  ifstream file(file_name, ios::binary);

  while (run) {
    uint64_t n_packet_bytes = 0;
    file.read(reinterpret_cast<char*>(&n_packet_bytes), sizeof(n_packet_bytes));
    file.read(buffer, n_packet_bytes);

    const int n_sent_bytes = send(socket_fd, buffer, n_packet_bytes, 0);
    if (n_sent_bytes < 0) {
      throw std::runtime_error(fmt::format("Cannot send on socket, error: {}", udp_port, errno));
    }

    if (n_sent_bytes != static_cast<int>(n_packet_bytes)) {
      throw std::runtime_error("Sent wrong number of bytes.");
    }

    n_packets += 1;

    if (n_packets % BARRIER_PACKET_COUNT == 0) {
      // Wait for all threads to reach the barrier
      std::lock_guard<std::mutex> lock(barrier_mutex);
      fmt::print("[Thread {}] Sent {} packets and reached barrier.\n", udp_port, n_packets);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(ms_delay));
  }
  file.close();
  delete[] buffer;
}

namespace {
std::tuple<utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_live_stream");
  program->add_argument("ms_delay").help("delay in milliseconds between packets");
  program->add_argument("folder_path").help("path to the folder containing .dat files");
  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("folder_path"), program->get<int>("ms_delay")};
}

} // namespace

int main(int argc, char** argv)
{
  const auto [config, folder_path, ms_delay] = read_arguments(argc, argv);
  std::vector<std::thread> threads;
  atomic_bool running(true);

  for (int i = 0; i < config.n_modules; i++) {
    auto udp_port = config.start_udp_port + i;
    threads.emplace_back(read_and_send, udp_port, std::ref(running), ms_delay, folder_path);
  }

  cin.get();
  running = false;

  for (auto& thread : threads)
    thread.join();
}
