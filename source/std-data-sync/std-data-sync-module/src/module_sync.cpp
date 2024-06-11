/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>
#include <thread>
#include <memory>
#include <optional>

#include <zmq.h>

#include "core_buffer/buffer_utils.hpp"
#include "detectors/common.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

#include "synchronizer.hpp"

using namespace std;

namespace {

template<typename FrameType>
void process_received_modules(const utils::DetectorConfig& config,
                              void* ctx,
                              std::shared_ptr<Synchronizer<FrameType>> syncer)
{
  utils::stats::SyncStatsCollector stats(config.detector_name, config.stats_collection_period);
  char meta_buffer_recv[DET_FRAME_STRUCT_BYTES];
  auto* common_frame = (FrameType*)(&meta_buffer_recv);

  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);

  while (true) {
    if (zmq_recv(receiver, meta_buffer_recv, DET_FRAME_STRUCT_BYTES, 0) > 0) {
      auto n_corrupted_images = syncer->process_image_metadata(*common_frame);
      stats.process(n_corrupted_images);
    }
    stats.print_stats();
  }
}

template<typename FrameType>
void send_synchronized_images(const utils::DetectorConfig& config,
                              void* ctx,
                              std::shared_ptr<Synchronizer<FrameType>> syncer)
{
  using namespace std::chrono_literals;
  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(utils::get_metadata_dtype(config));
  image_meta.set_height(config.image_pixel_height);
  image_meta.set_width(config.image_pixel_width);
  image_meta.set_size(utils::converted_image_n_bytes(config));
  image_meta.set_compression(std_daq_protocol::none);

  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);

  while (true) {
    if (auto meta = syncer->pop_next_full_image(); meta) {
      image_meta.set_image_id(meta->image_id);
      if (meta->n_missing_packets == 0)
        image_meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);
      else
        image_meta.set_status(std_daq_protocol::ImageMetadataStatus::missing_packets);

      std::string meta_buffer_send;
      image_meta.SerializeToString(&meta_buffer_send);
      zmq_send(sender, meta_buffer_send.c_str(), meta_buffer_send.size(), 0);
    }
    else
      std::this_thread::sleep_for(1ms);
  }
}

} // namespace

int main(int argc, char* argv[])
{
  static const std::string prog_name{"std_data_sync_module"};
  auto program = utils::create_parser(prog_name);
  program = utils::parse_arguments(std::move(program), argc, argv);
  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{prog_name, config.log_level};

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 4);

  auto syncer = std::make_shared<Synchronizer<CommonFrame>>(config.n_modules, config.module_sync_queue_size,
                                               utils::get_modules_mask(config));

  std::jthread processing_metadata_thread(process_received_modules<CommonFrame>, config, ctx, syncer);
  std::jthread sending_synchronized_images_thread(send_synchronized_images<CommonFrame>, config, ctx, syncer);
}
