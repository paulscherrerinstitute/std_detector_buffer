/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <zmq.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

#include "meta_buffer.hpp"

using namespace std::string_literals;

namespace {

constexpr auto zmq_io_threads = 1;

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_delay_filter");
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc and shared memory sources for ram_buffer")
      .default_value("image"s);

  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--source_suffix")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, source_suffix] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_delay_filter", config.log_level};

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-{}", config.detector_name, source_suffix);
  const auto stream_address = fmt::format("{}-delay-filter", config.detector_name);

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period,
                                          source_suffix);

  auto receiver = cb::Communicator{
      {source_name, utils::converted_image_n_bytes(config), utils::slots_number(config)},
      {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = buffer_utils::bind_socket(ctx, stream_address, ZMQ_PUB);

  meta_buffer metadata_buffer(utils::slots_number(config), config.delay_filter_timeout);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      metadata_buffer.push(meta);
      if (auto to_send = metadata_buffer.pop(); to_send.has_value()) {
        std::string meta_buffer_send;
        to_send.value().SerializeToString(&meta_buffer_send);
        zmq_send(sender_socket, meta_buffer_send.c_str(), meta_buffer_send.size(), 0);
      }
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
