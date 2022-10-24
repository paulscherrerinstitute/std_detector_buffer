/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "gigafrost.hpp"
#include "ram_buffer.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
} // namespace

void* bind_socket(void* ctx, const std::string& stream_address)
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
    fmt::print("Usage: std_stream_send_gf [detector_json_filename] [stream_address] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tstream_address: address to bind the output stream.\n"
               "\timage_half: 0 or 1 responsible for sending first or second part of image.\n");
    exit(-1);
  }
  return {buffer_utils::read_json_config(argv[1]), argv[2], argv[3]};
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, is_first_half] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto data_bytes_sent = converted_bytes / 2;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{
      {sync_name, sizeof(GFFrame), converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_socket(ctx, stream_address);

  ImageMetadata image_meta{};
  image_meta.dtype = ImageMetadataDtype::int16;
  image_meta.height = config.image_pixel_height;
  image_meta.width = config.image_pixel_width;

  GFFrame meta{};

  while (true) {
    auto [id, image_data] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    image_data = is_first_half ? image_data: image_data + data_bytes_sent;

    image_meta.id = id;
    image_meta.status = get_meta_data_status(meta.common.n_missing_packets);

    zmq_send(sender_socket, &image_meta, sizeof(image_meta), ZMQ_SNDMORE);
    zmq_send(sender_socket, image_data, data_bytes_sent, 0);
  }
  return 0;
}
