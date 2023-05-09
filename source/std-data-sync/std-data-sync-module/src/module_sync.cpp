/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>
#include <chrono>

#include <zmq.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "detectors/common.hpp"
#include "utils/args.hpp"
#include "utils/detector_config.hpp"
#include "utils/sync_stats_collector.hpp"
#include "utils/get_metadata_dtype.hpp"
#include "std_daq/image_metadata.pb.h"

#include "synchronizer.hpp"

using namespace std;
using namespace buffer_config;

namespace {

std::tuple<utils::DetectorConfig, std::chrono::milliseconds> read_arguments(int argc, char* argv[])
{
  using namespace std::chrono_literals;
  auto program = utils::create_parser("std_data_sync_module");
  program.add_argument("--drop_time")
      .default_value(300)
      .scan<'d', uint16_t>()
      .help("time in milliseconds after which the image is considered stale and dropped");

  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          1ms * program.get<uint16_t>("--drop_time")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, drop_time] = read_arguments(argc, argv);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);
  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);
  Synchronizer syncer(config.n_modules, SYNC_N_IMAGES_BUFFER, drop_time,
                      utils::get_modules_mask(config));

  utils::SyncStatsCollector stats("std_data_sync_module", config.detector_name);

  char meta_buffer_recv[DET_FRAME_STRUCT_BYTES];
  auto* common_frame = (CommonFrame*)(&meta_buffer_recv);

  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(utils::get_metadata_dtype(config));
  image_meta.set_height(config.image_pixel_height);
  image_meta.set_width(config.image_pixel_width);

  while (true) {
    zmq_recv(receiver, meta_buffer_recv, DET_FRAME_STRUCT_BYTES, 0);
    stats.processing_started();

    auto n_corrupted_images = syncer.process_image_metadata(*common_frame);

    for (auto m = syncer.pop_next_full_image(); m.image_id != INVALID_IMAGE_ID;
         m = syncer.pop_next_full_image())
    {
      image_meta.set_image_id(m.image_id);
      if (m.n_missing_packets == 0)
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
