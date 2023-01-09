/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "eiger.hpp"
#include "ram_buffer.hpp"
#include "utils/basic_stats_collector.hpp"

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

std::tuple<buffer_utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  if (argc != 3) {
    fmt::print("Usage: std_stream_send_eg [detector_json_filename] [stream_address] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tstream_address: address to bind the output stream.\n");
    exit(-1);
  }
  return {buffer_utils::read_json_config(argv[1]), argv[2]};
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const auto data_bytes =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8u;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{{sync_name, data_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_sender_socket(ctx, stream_address);
  utils::BasicStatsCollector stats("std_stream_send_eg", config.detector_name);

  ImageMetadata meta{};

  while (true) {
    auto [id, image_data] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    stats.processing_started();

    zmq_send(sender_socket, &meta, sizeof(meta), ZMQ_SNDMORE);
    zmq_send(sender_socket, image_data, data_bytes, 0);
    stats.processing_finished();
  }
  return 0;
}