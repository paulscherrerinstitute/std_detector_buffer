/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <zmq.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/buffer_config.hpp"
#include "detectors/common.hpp"

#include "synchronizer.hpp"
#include "sync_stats.hpp"
#include "image_metadata.pb.h"

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

  char meta_buffer_recv[DET_FRAME_STRUCT_BYTES];
  auto* common_frame = (CommonFrame*)(&meta_buffer_recv);

  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(std_daq_protocol::ImageMetadataDtype::uint16);
  image_meta.set_height(config.image_pixel_height);
  image_meta.set_width(config.image_pixel_width);

  while (true) {
    zmq_recv(receiver, meta_buffer_recv, DET_FRAME_STRUCT_BYTES, 0);

    auto [cached_meta, n_corrupted_images] = syncer.process_image_metadata(*common_frame);

    if (cached_meta.image_id == INVALID_IMAGE_ID) {
      continue;
    }

    image_meta.set_image_id(common_frame->image_id);
    if (common_frame->n_missing_packets == 0) {
      image_meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);
    } else {
      image_meta.set_status(std_daq_protocol::ImageMetadataStatus::missing_packets);
    }

    std::string meta_buffer_send;
    image_meta.SerializeToString(&meta_buffer_send);
    zmq_send(sender, meta_buffer_send.c_str(), meta_buffer_send.size(), 0);

    stats.record_stats(n_corrupted_images);
  }
}
