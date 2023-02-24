/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "detectors/gigafrost.hpp"
#include "std_daq/image_metadata.pb.h"
#include "utils/args.hpp"

#include "sender_stats_collector.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
constexpr auto zmq_success = 0;
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

std::tuple<buffer_utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_stream_send_gf");
  program.add_argument("stream_address").help("address to bind the input stream");
  program.add_argument("image_part")
      .help("0..7 responsible for sending n-th part of image")
      .scan<'d', int>();

  program = utils::parse_arguments(program, argc, argv);

  return {buffer_utils::read_json_config(program.get("detector_json_filename")),
          program.get("stream_address"), program.get<int>("image_part")};
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, image_part] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto start_index = image_part * gf::max_single_sender_size;
  if (converted_bytes <= start_index) return 0;
  const auto data_bytes_sent = std::min(converted_bytes - start_index, gf::max_single_sender_size);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{{sync_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_sender_socket(ctx, stream_address);
  gf::send::SenderStatsCollector stats(config.detector_name, image_part);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      stats.processing_started();

      meta.ParseFromArray(buffer, n_bytes);
      auto image_data = receiver.get_data(meta.image_id());

      std::size_t zmq_failed = zmq_success == zmq_send(sender_socket, buffer, n_bytes, ZMQ_SNDMORE);
      zmq_failed +=
          zmq_success == zmq_send(sender_socket, image_data + start_index, data_bytes_sent, 0);

      stats.processing_finished(zmq_failed);
    }
  }
  return 0;
}
