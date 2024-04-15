/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>
#include <thread>
#include <memory>

#include <zmq.h>

#include "core_buffer/buffer_utils.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

#include "synchronizer.hpp"

using namespace std;

namespace {

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_sync_metadata");
  program->add_argument("metadata_stream_address").help("address to connect to metadata stream");
  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("metadata_stream_address")};
}

void* zmq_socket_connect(void* ctx, const std::string& stream_address)
{
  void* socket = buffer_utils::create_socket(ctx, ZMQ_SUB);
  if (zmq_connect(socket, stream_address.c_str())) throw std::runtime_error(zmq_strerror(errno));
  return socket;
}

void process_incoming_metadata_stream(const utils::DetectorConfig& config,
                                      const std::string& metadata_stream_address,
                                      void* ctx,
                                      std::shared_ptr<Synchronizer> syncer)
{
  utils::stats::QueueStatsCollector stats(config.detector_name, config.stats_collection_period);

  char buffer[512];
  std_daq_protocol::ImageMetadata image_meta;

  auto metadata_socket = zmq_socket_connect(ctx, metadata_stream_address);
  while (true) {
    if (auto n_bytes = zmq_recv(metadata_socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
      image_meta.ParseFromArray(buffer, n_bytes);
      bool dropped = syncer->add_metadata(image_meta);
      stats.process(dropped);
      stats.update_queue_length(syncer->get_queue_length());
    }
    stats.print_stats();
  }
}

void process_incoming_image_stream(const utils::DetectorConfig& config,
                                   void* ctx,
                                   std::shared_ptr<Synchronizer> syncer)
{
  auto receiver = buffer_utils::bind_socket(ctx, config.detector_name + "-sync", ZMQ_PULL);

  char buffer[512];
  std_daq_protocol::ImageMetadata image_meta;

  while (true) {
    if (auto n_bytes = zmq_recv(receiver, buffer, sizeof(buffer), 0); n_bytes > 0) {
      image_meta.ParseFromArray(buffer, n_bytes);
      syncer->add_received_image(image_meta.image_id());
    }
  }
}

void send_synchronized_images(const utils::DetectorConfig& config,
                              void* ctx,
                              std::shared_ptr<Synchronizer> syncer)
{
  auto sender = buffer_utils::bind_socket(ctx, config.detector_name + "-image", ZMQ_PUB);

  while (true) {
    if (auto meta = syncer->get_next_received_image(); meta.has_value()) {
      std::string meta_buffer_send;
      meta->SerializeToString(&meta_buffer_send);
      zmq_send(sender, meta_buffer_send.c_str(), meta_buffer_send.size(), 0);
    }
  }
}
} // namespace

int main(int argc, char* argv[])
{
  static const std::string prog_name{"std_data_sync_metadata"};
  const auto [config, metadata_stream_address] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{prog_name, config.log_level};

  if(!config.sender_sends_full_images) return 0;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, 4);

  auto syncer = std::make_shared<Synchronizer>(2000);

  std::jthread processing_metadata_thread(process_incoming_metadata_stream, config,
                                          metadata_stream_address, ctx, syncer);
  std::jthread processing_images_thread(process_incoming_image_stream, config, ctx, syncer);
  std::jthread sending_synchronized_images_thread(send_synchronized_images, config, ctx, syncer);

  return 0;
}
