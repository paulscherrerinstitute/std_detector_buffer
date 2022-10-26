/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "gigafrost.hpp"
#include "ram_buffer.hpp"
#include "synchronizer.hpp"

using namespace buffer_utils;

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
} // namespace

void* zmq_socket_bind(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_PUB);

  //TODO: setup correctly the
//  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
//    throw std::runtime_error(zmq_strerror(errno));
//
//  const int linger = 0;
//  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
//    throw std::runtime_error(zmq_strerror(errno));
//
//  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

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
  int64_t receive_more;
  size_t sz = sizeof(receive_more);
  return zmq_getsockopt(socket, ZMQ_RCVMORE, &receive_more, &sz) == 0 && receive_more &&
         zmq_recv(socket, &buffer, size, 0) == 0;
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address_first, stream_address_second] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto data_bytes_sent = converted_bytes / 2;

  ssrg::Synchronizer sync{10};
  ImageMetadata image_meta{};
  auto image_meta_as_span = std::span<char>((char*)&image_meta, sizeof(ImageMetadata));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto sender = cb::Communicator{
      {sync_name, sizeof(GFFrame), converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUB}};

  auto sockets = std::array{zmq_socket_bind(ctx, stream_address_first),
                            zmq_socket_bind(ctx, stream_address_second)};

  while (true) {
    for(auto i = 0u; i < sockets.size(); i++)
    {
      if (zmq_recv(sockets[i], &image_meta, sizeof(image_meta), 0) == 0) {
        auto* data = sender.get_data(image_meta.id) + (i * data_bytes_sent);
        if (received_successfully_data(sockets[i], data, data_bytes_sent) &&
            sync.is_ready_to_send(image_meta.id))
          sender.send(image_meta.id, image_meta_as_span, data);
      }
    }
  }
  return 0;
}
