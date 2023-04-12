/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "detectors/gigafrost.hpp"
#include "std_daq/image_metadata.pb.h"
#include "utils/args.hpp"
#include "utils/basic_stats_collector.hpp"
#include "utils/detector_config.hpp"
#include "utils/get_metadata_dtype.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

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

std::tuple<utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_live_stream");
  program.add_argument("stream_address").help("address to bind the output stream");
  program.add_argument("-d", "--data_rate").help("rate in [Hz]").action([](const std::string& arg) {
    if (auto value = std::stoi(arg); value < 1 || value > 1000)
      throw std::runtime_error("Unsupported data_rate! [1-100 Hz] is valid");
    else
      return value;
  });

  program.add_argument("-f", "--forward")
      .help("forward all images")
      .default_value(false)
      .implicit_value(true);

  program = utils::parse_arguments(program, argc, argv);

  if (program.is_used("--data_rate") && program.is_used("--forward"))
    throw std::runtime_error(fmt::format("--data_rate and --forward can't be defined together"));

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address"),
          program["--forward"] == true ? 0 : program.get<int>("data_rate")};
}

std::size_t converted_image_n_bytes(const utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost")
    return gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  if (config.detector_type == "eiger" || config.detector_type == "pco")
    return config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  throw std::runtime_error("Unsupported detector_type!\n");
}

size_t update_size_for_pco(const std_daq_protocol::ImageMetadata& meta)
{
  return meta.height() * meta.width() * utils::get_bytes_from_metadata_dtype(meta.dtype());
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address, data_rate] = read_arguments(argc, argv);
  const auto data_period = data_rate == 0 ? 0ms : 1000ms / data_rate;
  auto converted_bytes = converted_image_n_bytes(config);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-image", config.detector_name);

  auto receiver =
      cb::Communicator{{source_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                       {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = bind_sender_socket(ctx, stream_address);
  utils::BasicStatsCollector stats("std_live_stream", config.detector_name);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  auto prev_sent_time = std::chrono::steady_clock::now();

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      auto image_data = receiver.get_data(meta.image_id());

      if (const auto now = std::chrono::steady_clock::now(); prev_sent_time + data_period < now) {
        stats.processing_started();
        prev_sent_time = now;
        if (config.detector_type == "pco") converted_bytes = update_size_for_pco(meta);

        auto encoded = fmt::format(
            R"({{"htype":"array-1.0", "shape":[{},{}], "type":"{}", "frame":{}}})", meta.height(),
            meta.width(), utils::get_array10_type(meta.dtype()), meta.image_id());
        auto encoded_c = encoded.c_str();

        zmq_send(sender_socket, encoded_c, encoded.length(), ZMQ_SNDMORE);
        zmq_send(sender_socket, image_data, converted_bytes, 0);
        stats.processing_finished();
      }
    }
  }
  return 0;
}
