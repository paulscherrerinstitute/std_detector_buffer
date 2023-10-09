/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <zmq.h>

#include "core_buffer/buffer_utils.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

#include "synchronizer.hpp"
#include "queue_len_stats_collector.hpp"

using namespace std;

int main(int argc, char* argv[])
{
  static const std::string prog_name{"std_data_sync_stream"};
  auto program = utils::create_parser(prog_name);
  program = utils::parse_arguments(program, argc, argv);
  const auto config = utils::read_config_from_json_file(program.get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{prog_name, config.log_level};

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 1);

  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);
  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);
  const int parts = static_cast<int>(utils::number_of_senders(config));

  Synchronizer syncer(parts, 1000);

  sdss::QueueStatsCollector stats(config.detector_name, config.stats_collection_period);

  char buffer[512];
  std_daq_protocol::ImageMetadata image_meta;

  while (true) {
    if (auto n_bytes = zmq_recv(receiver, buffer, sizeof(buffer), 0); n_bytes > 0) {
        image_meta.ParseFromArray(buffer, n_bytes);

      auto [cached_id, n_corrupted_images] = syncer.process_image_metadata(image_meta.image_id());
      if (cached_id != INVALID_IMAGE_ID) zmq_send(sender, buffer, n_bytes, 0);
      stats.process(n_corrupted_images, syncer.get_queue_length());
    }
    stats.print_stats();
  }
}
