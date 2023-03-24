/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
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

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_stream_receive_eg");
  program.add_argument("stream_address").help("address to bind the input stream");
  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address")};
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
  const auto [config, stream_address] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const auto data_bytes =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8u;

  eg::rec::ReceiverStatsCollector stats(config.detector_name);

  ImageMetadata image_meta{};
  auto image_meta_as_span = std::span<char>((char*)&image_meta, sizeof(ImageMetadata));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto sender = cb::Communicator{{sync_name, data_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sync_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  auto socket = zmq_socket_bind(ctx, stream_address);
  while (true) {
    unsigned int zmq_fails = 0;
    stats.processing_started();
    if (zmq_recv(socket, &image_meta, sizeof(image_meta), 0) > 0) {
      char* data = sender.get_data(image_meta.id);
      if (received_successfully_data(socket, data, data_bytes))
        sender.send(image_meta.id, image_meta_as_span, data);
      else
        zmq_fails++;
    }
    else
      zmq_fails++;
    stats.processing_finished(zmq_fails);
  }
  return 0;
}
