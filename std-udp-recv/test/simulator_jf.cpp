#include <cstdint>
#include <iostream>
#include "../include/UdpRecvConfig.hpp"
#include <netinet/in.h>
#include <unistd.h>
#include "mock/udp.hpp"


#include "jungfrau.hpp"

const int MAX_IMAGE_ID = 10000;

using namespace std;

int main(int argc, char** argv)
{

  if (argc != 4) {
    cout << endl;
    cout << "Usage: std_udp_sim_jf [detector_json_filename] [ms_delay]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << "\tms_delay: delay in milliseconds between images." << endl;
    cout << endl;
    exit(-1);
  }

  const auto config = UdpRecvConfig::from_json_file(string(argv[1]));
  const int ms_delay = stoi(argv[3]);

  if (DETECTOR_TYPE != config.detector_type) {
    throw runtime_error("Simulator for " + DETECTOR_TYPE + ", config for " + config.detector_type);
  }

  int sockets[config.n_modules];
  sockaddr_in send_address[config.n_modules];

  for (int i_module = 0; i_module < config.n_modules; i_module++) {
    send_address[i_module] = get_server_address(config.start_udp_port + i_module);

    sockets[i_module] = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockets[i_module] < 0) {
      throw runtime_error("Cannot bind UDP socket.");
    }
  }

  uint64_t image_id = 0;
  while (true) {
    for (size_t i_packet = 0; i_packet < N_PACKETS_PER_FRAME; i_packet++) {
      for (int i_module = 0; i_module < config.n_modules; i_module++) {

        JFUdpPacket send_udp_buffer = {};
        send_udp_buffer.packetnum = i_packet;

        send_udp_buffer.framenum = image_id + 100;
        send_udp_buffer.bunchid = image_id;

        ::sendto(sockets[i_module], &send_udp_buffer, BYTES_PER_PACKET, 0,
                 (sockaddr*)&send_address[i_module], sizeof(sockaddr_in));
      }
    }

    cout << "Sent image_id " << image_id << endl;
    // 10Hz == 100ms between images
    usleep(ms_delay * 1000);

    image_id = ++image_id % MAX_IMAGE_ID;
  }
}