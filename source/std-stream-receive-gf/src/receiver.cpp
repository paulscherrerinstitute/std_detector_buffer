/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/gigafrost.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "synchronizer.hpp"
#include "receiver_stats_collector.hpp"

using namespace buffer_utils;

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

std::tuple<DetectorConfig, std::string, std::string> read_arguments(int argc, char* argv[])
{
  if (argc != 4) {
    fmt::print("Usage: std_stream_receive_gf [detector_json_filename]"
               " [stream_address_first_half] [stream_address_second_half] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tstream_address_first_half: address to bind the input stream"
               " - first half of image.\n"
               "\tstream_address_second_half:address to bind the input stream"
               " - first half of image.\n");
    exit(-1);
  }
  return {read_json_config(argv[1]), argv[2], argv[3]};
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
  const auto [config, stream_address_first, stream_address_second] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto data_bytes_sent = converted_bytes / 2;

  gf::rec::Synchronizer sync{10};
  gf::rec::ReceiverStatsCollector stats(config.detector_name, sync);

  ImageMetadata image_meta{};
  auto image_meta_as_span = std::span<char>((char*)&image_meta, sizeof(ImageMetadata));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto sender = cb::Communicator{
      {sync_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  auto sockets = std::array{zmq_socket_bind(ctx, stream_address_first),
                            zmq_socket_bind(ctx, stream_address_second)};
  while (true) {
    unsigned int zmq_fails = 0;
    stats.processing_started();
    for (auto i = 0u; i < sockets.size(); i++) {
      if (zmq_recv(sockets[i], &image_meta, sizeof(image_meta), 0) > 0) {
        char* data = sender.get_data(image_meta.id);
        if (received_successfully_data(sockets[i], data + (i * data_bytes_sent), data_bytes_sent)) {
          if (sync.is_ready_to_send(image_meta.id))
            sender.send(image_meta.id, image_meta_as_span, data);
        }
        else
          zmq_fails++;
      }
      else
        zmq_fails++;
    }
    stats.processing_finished(zmq_fails);
  }
  return 0;
}
