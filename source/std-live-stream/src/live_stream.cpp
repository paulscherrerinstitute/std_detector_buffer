/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;

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

std::tuple<utils::DetectorConfig, std::string, std::string, int> read_arguments(int argc,
                                                                                char* argv[])
{
  auto program = utils::create_parser("std_live_stream");
  program.add_argument("stream_address").help("address to bind the output stream");
  program.add_argument("-d", "--data_rate").help("rate in [Hz]").action([](const std::string& arg) {
    if (auto value = std::stoi(arg); value < 1 || value > 1000)
      throw std::runtime_error("Unsupported data_rate! [1-100 Hz] is valid");
    else
      return value;
  });
  program.add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer - default \"image\"")
      .default_value("image"s);

  program.add_argument("-f", "--forward")
      .help("forward all images")
      .default_value(false)
      .implicit_value(true);

  program = utils::parse_arguments(program, argc, argv);

  if (program.is_used("--data_rate") && program.is_used("--forward"))
    throw std::runtime_error(fmt::format("--data_rate and --forward can't be defined together"));

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address"), program.get("--source_suffix"),
          program["--forward"] == true ? 0 : program.get<int>("data_rate")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address, source_suffix, data_rate] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_live_stream", config.log_level};
  const auto data_period = data_rate == 0 ? 0ms : 1000ms / data_rate;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-{}", config.detector_name, source_suffix);

  auto receiver = cb::Communicator{
      {source_name, utils::converted_image_n_bytes(config), buffer_config::RAM_BUFFER_N_SLOTS},
      {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = bind_sender_socket(ctx, stream_address);
  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  auto prev_sent_time = std::chrono::steady_clock::now();

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      auto image_data = receiver.get_data(meta.image_id());

      if (const auto now = std::chrono::steady_clock::now(); prev_sent_time + data_period < now) {
        prev_sent_time = now;

        auto encoded = fmt::format(
            R"({{"htype":"array-1.0", "shape":[{},{}], "type":"{}", "frame":{}}})", meta.height(),
            meta.width(), utils::get_array10_type(meta.dtype()), meta.image_id());
        auto encoded_c = encoded.c_str();

        zmq_send(sender_socket, encoded_c, encoded.length(), ZMQ_SNDMORE);
        zmq_send(sender_socket, image_data, meta.size(), 0);
      }
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
