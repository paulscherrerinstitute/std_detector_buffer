/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <string>

#include <fmt/core.h>
#include <zmq.h>

#include "utils/utils.hpp"
#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"

#include <core_buffer/buffer_utils.hpp>

using namespace std;

int main(int argc, char* argv[])
{
  const std::string program_name{"std_chunk_sender"};
  auto program = utils::create_parser(program_name);

  program = utils::parse_arguments(std::move(program), argc, argv);
  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{program_name, config.log_level};

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);
  const auto source_name = fmt::format("{}-image", config.detector_name);

  auto ctx = zmq_ctx_new();

  auto receiver = cb::Communicator{
        {source_name, utils::converted_image_n_bytes(config), utils::slots_number(config)},
        {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PULL}};

  char buffer[512];

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      stats.print_stats();
    }
  }
  return 0;
}
