/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <zmq.h>
#include <buffer_utils.hpp>
#include <SyncStats.hpp>

#include "core_buffer/communicator.hpp"
#include "ZmqPulseSyncReceiver.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    cout << endl;
    cout << "Usage: std_udp_sync [detector_json_filename]" << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << endl;

    exit(-1);
  }
  const auto config = buffer_utils::read_json_config(std::string(argv[1]));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

  ZmqPulseSyncReceiver receiver(ctx, config.detector_name, config.n_modules);
  socket_ = buffer_utils::bind_socket(ctx_, detector_name + "-sync", ZMQ_PULL);
  auto receiver =
      cb::Communicator{{fmt::format("{}-{}", config.detector_name, "sync"), sizeof(GFFrame),
                        module_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                       {ctx, cb::CONN_TYPE_BIND, ZMQ_PULL}};

  const auto sync_name = config.detector_name + "-image";

  auto sender = cb::Communicator{
      {sync_name, sizeof(GFFrame), converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  SyncStats stats(config.detector_name, STATS_TIME);

  while (true) {
    auto meta = receiver.get_next_image_id();
    std::cout << meta.image_id << std::endl;

    zmq_send(sender, &meta.image_id, sizeof(meta.image_id), 0);

    stats.record_stats(meta.n_lost_pulses);
  }
}
