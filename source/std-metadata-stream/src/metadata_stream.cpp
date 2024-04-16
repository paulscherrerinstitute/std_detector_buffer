/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "utils/utils.hpp"

using namespace std::string_literals;

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 50000;

std::tuple<utils::DetectorConfig, std::string, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_metadata_stream");
  program->add_argument("stream_address").help("address to bind the output stream");
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for data stream - default \"image\"")
      .default_value("image"s);
  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"), program->get("--source_suffix")};
}

void* bind_sender_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_PUB);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));
  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address, suffix] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_metadata_stream", config.log_level};

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);
  const auto source_name = fmt::format("{}-{}", config.detector_name, suffix);

  auto receiver = cb::Communicator{
      {source_name, utils::converted_image_n_bytes(config), buffer_config::RAM_BUFFER_N_SLOTS},
      {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_sender_socket(ctx, stream_address);

  char buffer[512];
  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      zmq_send(sender_socket, buffer, n_bytes, 0);
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
