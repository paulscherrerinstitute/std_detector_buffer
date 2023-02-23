/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <zmq.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/buffer_config.hpp"
#include "detectors/common.hpp"
#include "utils/args.hpp"
#include "std_daq/image_metadata.pb.h"

#include "synchronizer.hpp"
#include "sync_stats_collector.hpp"

using namespace std;
using namespace buffer_config;

int main(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_sync");
  program = utils::parse_arguments(program, argc, argv);
  const auto config = buffer_utils::read_json_config(program.get("detector_json_filename"));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);
  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);
  Synchronizer syncer(config.n_modules, SYNC_N_IMAGES_BUFFER);

  gf::sync::SyncStatsCollector stats(config.detector_name);

  char meta_buffer_recv[DET_FRAME_STRUCT_BYTES];
  auto* common_frame = (CommonFrame*)(&meta_buffer_recv);

  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(std_daq_protocol::ImageMetadataDtype::uint16);
  image_meta.set_height(config.image_pixel_height);
  image_meta.set_width(config.image_pixel_width);

  while (true) {
    zmq_recv(receiver, meta_buffer_recv, DET_FRAME_STRUCT_BYTES, 0);
    stats.processing_started();

    auto [cached_id, n_corrupted_images] =
        syncer.process_image_metadata(common_frame->image_id, common_frame->module_id);

    if (cached_id != INVALID_IMAGE_ID) {
      image_meta.set_image_id(common_frame->image_id);
      if (common_frame->n_missing_packets == 0)
        image_meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);
      else
        image_meta.set_status(std_daq_protocol::ImageMetadataStatus::missing_packets);

      std::string meta_buffer_send;
      image_meta.SerializeToString(&meta_buffer_send);
      zmq_send(sender, meta_buffer_send.c_str(), meta_buffer_send.size(), 0);
    }
    stats.processing_finished(n_corrupted_images);
  }
}
