/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer_common/buffer_handler.hpp"
#include "std_buffer_common/redis_handler.hpp"
#include "std_buffer/buffered_metadata.pb.h"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
constexpr auto zmq_success = 0;

auto calculate_size(const std_daq_protocol::ImageMetadata* meta)
{
  return meta->height() * meta->width() * utils::get_bytes_from_metadata_dtype(meta->dtype());
}

} // namespace

std::tuple<utils::DetectorConfig, std::string, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_buffer_writer");
  program.add_argument("--root_dir").help("Root directory where files will be stored").required();
  program.add_argument("--db_address")
      .help("Address of Redis API compatible database to connect")
      .required();
  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("--db_address"), program.get("--root_dir")};
}

int main(int argc, char* argv[])
{
  const auto [config, db_address, root_dir] = read_arguments(argc, argv);
  sbc::RedisHandler sender(config.detector_name, db_address);
  sbc::BufferHandler writer(root_dir + config.detector_name, config.bit_depth / 8);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const std::size_t max_data_bytes =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8u;

  auto receiver = cb::Communicator{{sync_name, max_data_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  char buffer[512];
  std_daq_protocol::BufferedMetadata buffered_meta;
  auto* meta = buffered_meta.mutable_metadata();

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta->ParseFromArray(buffer, n_bytes);

      auto* image_data = receiver.get_data(meta->image_id());
      const auto size = calculate_size(meta);
      const auto [offset, compressed_size] =
          writer.write(meta->image_id(), std::span<char>(image_data, size));

      buffered_meta.set_offset(offset);
      buffered_meta.set_size(compressed_size);
      sender.send(meta->image_id(), buffered_meta);
    }
  }
  return 0;
}
