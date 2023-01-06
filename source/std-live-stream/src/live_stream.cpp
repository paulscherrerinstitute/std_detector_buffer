/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "gigafrost.hpp"
#include "ram_buffer.hpp"
#include "utils/basic_stats_collector.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
} // namespace

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

std::tuple<buffer_utils::DetectorConfig, std::string, bool> read_arguments(int argc, char* argv[])
{
  if (argc != 4) {
    fmt::print("Usage: std_live_stream_gf [detector_json_filename] [stream_address] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tstream_address: address to bind the output stream.\n"
               "\tdata_rate: 1-100 Hz.\n");
    exit(-1);
  }
  auto data_rate = std::stoi(argv[3]);
  if (data_rate > 100) throw std::runtime_error("Unsupported data_rate! [1-100 Hz] is valid");
  return {buffer_utils::read_json_config(argv[1]), argv[2], data_rate};
}

std::size_t converted_image_n_bytes(const buffer_utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost")
    return gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  if (config.detector_type == "eiger")
    return config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  return 0;
}

std::string get_data_type(const buffer_utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost") return "uint16";
  if (config.detector_type == "eiger") {
    switch (config.bit_depth) {
    case 8:
      return "uint8";
    case 16:
      return "uint16";
    case 32:
      return "uint32";
    default:
      return "uint8";
    }
  }
  return "uint8";
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, data_rate] = read_arguments(argc, argv);
  const auto data_period = 1000ms / data_rate;
  const auto converted_bytes = converted_image_n_bytes(config);
  const auto data_type = get_data_type(config);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{{fmt::format("{}-image", config.detector_name), converted_bytes,
                                    buffer_config::RAM_BUFFER_N_SLOTS},
                                   {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = bind_sender_socket(ctx, stream_address);
  utils::BasicStatsCollector stats("std_live_stream", config.detector_name);

  ImageMetadata meta{};
  auto prev_sent_time = std::chrono::steady_clock::now();

  while (true) {
    auto [_, image_data] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));

    if (const auto now = std::chrono::steady_clock::now(); prev_sent_time + data_period < now) {
      stats.processing_started();
      prev_sent_time = now;

      auto encoded =
          fmt::format(R"({{"htype":"array-1.0", "shape":[{},{}], "type":"{}", "frame":{}}})",
                      config.image_pixel_width, config.image_pixel_height, data_type, meta.id);
      auto encoded_c = encoded.c_str();

      zmq_send(sender_socket, encoded_c, encoded.length(), ZMQ_SNDMORE);
      zmq_send(sender_socket, image_data, converted_bytes, 0);
      stats.processing_finished();
    }
  }
  return 0;
}
