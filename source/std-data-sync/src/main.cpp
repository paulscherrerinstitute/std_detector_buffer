/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <zmq.h>
#include <buffer_utils.hpp>
#include <SyncStats.hpp>
#include <fmt/core.h>

#include "common.hpp"
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

  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);
  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);

  SyncStats stats(config.detector_name, STATS_TIME);

  char meta_buffer[DET_FRAME_STRUCT_BYTES];
  auto* meta = (CommonFrame*)(&meta_buffer);

  while (true) {
    auto status = zmq_recv(receiver, meta_buffer, DET_FRAME_STRUCT_BYTES, 0);
    std::cout << meta->image_id << std::endl;
    fmt::print("{}: module{}", meta->image_id, meta->module_id);

    zmq_send(sender, &meta->image_id, sizeof(meta->image_id), 0);

    stats.record_stats(0);
  }
}
