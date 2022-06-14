#include <iostream>
#include <stdexcept>
#include <zmq.h>
#include <RamBuffer.hpp>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "FrameUdpReceiver.hpp"
#include "buffer_utils.hpp"
#include "FrameStats.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace buffer_utils;

int main(int argc, char* argv[])
{

  if (argc != 3) {
    cout << endl;
    cout << "Usage: std_udp_recv_jf [detector_json_filename] [module_id]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << "\tmodule_id: id of the module for this process." << endl;
    cout << endl;

    exit(-1);
  }

  const auto config = read_json_config(string(argv[1]));
  const int module_id = atoi(argv[2]);

  const auto udp_port = config.start_udp_port + module_id;
  FrameUdpReceiver receiver(udp_port, module_id);
  RamBuffer buffer(config.detector_name, sizeof(jungfrau_packet), DATA_BYTES_PER_FRAME,
                   config.n_modules);
  FrameStats stats(config.detector_name, module_id, N_PACKETS_PER_FRAME, STATS_TIME);

  auto ctx = zmq_ctx_new();
  auto socket = bind_socket(ctx, config.detector_name, to_string(module_id));

  ModuleFrame meta;
  char* data = new char[MODULE_N_BYTES];

  uint64_t pulse_id_previous = 0;
  uint64_t frame_index_previous = 0;

  while (true) {

    auto pulse_id = receiver.get_frame_from_udp(meta, data);

    if ((meta.frame_index != (frame_index_previous + 1)) || ((pulse_id - pulse_id_previous) <= 0) ||
        ((pulse_id - pulse_id_previous) > 1000))
    {}
    else {

      buffer.write(meta.frame_index, (char*)(&meta), data);

      zmq_send(socket, &pulse_id, sizeof(pulse_id), 0);
    }

    stats.record_stats(N_PACKETS_PER_FRAME - meta.n_recv_packets);

    pulse_id_previous = pulse_id;
    frame_index_previous = meta.frame_index;
  }

  delete[] data;
}
