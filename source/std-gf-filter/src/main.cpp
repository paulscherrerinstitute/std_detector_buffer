/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <zmq.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

#include "filter_stats_collection.hpp"

using namespace std::string_literals;

namespace {

constexpr auto zmq_io_threads = 1;

std::tuple<utils::DetectorConfig, std::string, bool> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_gf_filter");
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc and shared memory sources for ram_buffer")
      .default_value("image"s);
  program->add_argument("-n", "--no_filter").help("forward all images").flag();

  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--source_suffix"), program->get<bool>("--no_filter")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, source_suffix, always_forward] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_gf_filter", config.log_level};

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-{}", config.detector_name, source_suffix);
  const auto stream_address = fmt::format("{}-filter", config.detector_name);

  auto receiver = cb::Communicator{
      {source_name, utils::converted_image_n_bytes(config), config.full_image_ram_buffer_slots},
      {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = buffer_utils::bind_socket(ctx, stream_address, ZMQ_PUB);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  FilterStatsCollector stats(config.detector_name, config.stats_collection_period, source_suffix);

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (always_forward || meta.gf().store_image()) {
        stats.forward();
        zmq_send(sender_socket, buffer, n_bytes, 0);
      }
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
