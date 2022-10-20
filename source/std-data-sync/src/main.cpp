/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <zmq.h>
#include <buffer_utils.hpp>
#include <sync_stats.hpp>
#include <fmt/core.h>

#include "common.hpp"
#include "buffer_config.hpp"
#include "synchronizer.hpp"

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
  Synchronizer syncer(config.n_modules, SYNC_N_IMAGES_BUFFER);

  SyncStats stats(config.detector_name, STATS_TIME);

  char meta_buffer[DET_FRAME_STRUCT_BYTES];
  auto* meta = (CommonFrame*)(&meta_buffer);

  while (true) {
    zmq_recv(receiver, meta_buffer, DET_FRAME_STRUCT_BYTES, 0);

    auto [cached_meta, n_corrupted_images] = syncer.process_image_metadata(*meta);

    fmt::print("{}: module{}\n", meta->image_id, meta->module_id);
    std::fflush(stdout);

    zmq_send(sender, &(cached_meta.image_id), sizeof(cached_meta.image_id), 0);

    stats.record_stats(n_corrupted_images);
  }
}
