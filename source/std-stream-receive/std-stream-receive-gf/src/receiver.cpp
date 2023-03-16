/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "detectors/gigafrost.hpp"
#include "std_daq/image_metadata.pb.h"
#include "utils/args.hpp"
#include "utils/detector_config.hpp"

#include "receiver_stats_collector.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
} // namespace

void* zmq_socket_bind(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_SUB);
  if (zmq_connect(socket, stream_address.c_str())) throw std::runtime_error(zmq_strerror(errno));
  if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0)) throw std::runtime_error(zmq_strerror(errno));
  return socket;
}

std::tuple<utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_stream_receive_gf");
  program.add_argument("stream_address").help("address to bind input stream");
  program.add_argument("image_part")
      .help("0..7 responsible for sending n-th part of image")
      .scan<'d', int>();
  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address"), program.get<int>("image_part")};
}

bool received_successfully_data(void* socket, char* buffer, std::size_t size)
{
  int receive_more;
  size_t sz = sizeof(receive_more);
  return zmq_getsockopt(socket, ZMQ_RCVMORE, &receive_more, &sz) != -1 &&
         zmq_recv(socket, buffer, size, ZMQ_DONTWAIT) > 0;
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, image_part] = read_arguments(argc, argv);
  const auto image_name = fmt::format("{}-image", config.detector_name);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto start_index = image_part * gf::max_single_sender_size;
  if (converted_bytes <= start_index) return 0;
  const auto data_bytes_sent = std::min(converted_bytes - start_index, gf::max_single_sender_size);

  gf::rec::ReceiverStatsCollector stats(config.detector_name);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto sender = cb::Communicator{{image_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  auto socket = zmq_socket_bind(ctx, stream_address);
  while (true) {
    unsigned int zmq_fails = 0;
    meta.set_image_id(0);
    stats.processing_started();
    if (auto n_bytes = zmq_recv(socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      char* data = sender.get_data(meta.image_id());
      if (received_successfully_data(socket, data + start_index, data_bytes_sent))
        sender.send(meta.image_id(), std::span{buffer, static_cast<std::size_t>(n_bytes)}, nullptr);
      else
        zmq_fails++;
    }
    else
      zmq_fails++;
    stats.processing_finished(zmq_fails, meta.image_id());
  }
  return 0;
}
