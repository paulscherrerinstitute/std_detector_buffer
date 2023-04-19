/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_daq/buffered_metadata.pb.h"
#include "std_daq/image_metadata.pb.h"
#include "utils/args.hpp"
#include "utils/detector_config.hpp"
#include "utils/get_metadata_dtype.hpp"

#include "redis_sender.hpp"
#include "buffer_writer.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
constexpr auto zmq_success = 0;
} // namespace

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_buffer_writer");
  program.add_argument("db_address").help("Address of Redis API compatible database to connect");
  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("db_address")};
}

int main(int argc, char* argv[])
{
  const auto [config, db_address] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const std::size_t max_data_bytes =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8u;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{{sync_name, max_data_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  sbw::RedisSender redis_send(db_address);
  // TODO: should path be defined as parameter? or fixed with detector name
  sbw::BufferWriter writer("/tmp");

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  std_daq_protocol::BufferedMetadata buffered_meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);

      auto* image_data = receiver.get_data(meta.image_id());
      auto size = meta.height() * meta.width() * utils::get_bytes_from_metadata_dtype(meta.dtype());
      const auto offset = writer.write(meta.image_id(), std::span<char>(image_data, size));

      buffered_meta.set_offset(offset);
      buffered_meta.set_allocated_metadata(&meta);
      redis_send.set(meta.image_id(), buffered_meta);
    }
  }
  return 0;
}
