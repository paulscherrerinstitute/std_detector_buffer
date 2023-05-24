/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "detectors/common.hpp"
#include "std_buffer_common/redis_handler.hpp"
#include "std_buffer_common/buffer_handler.hpp"
#include "std_daq/buffered_metadata.pb.h"
#include "std_daq/image_metadata.pb.h"
#include "utils/args.hpp"
#include "utils/detector_config.hpp"
#include "utils/get_metadata_dtype.hpp"
#include "utils/image_size_calc.hpp"

namespace {

constexpr auto zmq_io_threads = 1;

struct arguments
{
  utils::DetectorConfig config;
  std::string root_dir;
  std::string db_address;
  uint64_t start_image_id;
  uint64_t end_image_id;
};

arguments read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_buffer_reader");
  program.add_argument("--root_dir").help("Root directory where files will be stored").required();
  program.add_argument("--db_address")
      .help("Address of Redis API compatible database to connect")
      .required();
  program.add_argument("--start_id").help("Starting ID for read").scan<'i', uint64_t>().required();
  program.add_argument("--end_id")
      .help("Root directory where files will be stored")
      .scan<'i', uint64_t>()
      .default_value(INVALID_IMAGE_ID);
  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("--root_dir"), program.get("--db_address"),
          program.get<uint64_t>("--start_id"), program.get<uint64_t>("--end_id")};
}

std::size_t get_size(const std_daq_protocol::ImageMetadata& data)
{
  return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
}

} // namespace

int main(int argc, char* argv[])
{
  const auto args = read_arguments(argc, argv);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto sync_name = fmt::format("{}-image", args.config.detector_name);
  const std::size_t max_data_bytes = utils::converted_image_n_bytes(args.config);
  auto sender = cb::Communicator{{sync_name, max_data_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sync_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  sbc::RedisHandler redis_handler(args.config.detector_name, args.db_address);
  sbc::BufferHandler reader(args.root_dir + args.config.detector_name);
  std_daq_protocol::BufferedMetadata buffered_meta;

  // Streaming images should only be started when someone starts listening
  sender.await_sub_connection();

  auto images = redis_handler.get_more_recent_image_ids(args.start_image_id, args.end_image_id);

  for (auto image : images) {
    redis_handler.receive(image, buffered_meta);
    if (auto size = get_size(buffered_meta.metadata()); size <= max_data_bytes) {
      reader.read(image, {sender.get_data(image), size}, buffered_meta.offset());

      std::string meta_buffer_send;
      buffered_meta.metadata().SerializeToString(&meta_buffer_send);
      sender.send(image, meta_buffer_send, nullptr);
      // TODO: delay should be more useful than this one
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
  return 0;
}
