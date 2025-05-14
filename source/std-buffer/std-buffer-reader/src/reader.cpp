/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <thread>
#include <chrono>
#include <ranges>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "detectors/common.hpp"
#include "std_buffer_common/redis_handler.hpp"
#include "std_buffer_common/buffer_handler.hpp"
#include "std_buffer/buffered_metadata.pb.h"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

namespace {

constexpr auto zmq_io_threads = 1;

struct arguments
{
  utils::DetectorConfig config;
  std::string root_dir;
  std::string db_address;
  uint64_t start_image_id;
  uint64_t end_image_id;
  uint64_t delay;
};

arguments read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_buffer_reader");
  program->add_argument("--root_dir").help("Root directory where files will be stored").required();
  program->add_argument("--db_address")
      .help("Address of Redis API compatible database to connect")
      .required();
  program->add_argument("--delay")
      .help("Delay in between sending loaded images.")
      .scan<'i', uint64_t>()
      .required();
  program->add_argument("--start_id").help("Starting ID for read").scan<'i', uint64_t>().required();
  program->add_argument("--end_id")
      .help("Root directory where files will be stored")
      .scan<'i', uint64_t>()
      .default_value(INVALID_IMAGE_ID);
  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--root_dir"),
          program->get("--db_address"),
          program->get<uint64_t>("--start_id"),
          program->get<uint64_t>("--end_id"),
          program->get<uint64_t>("--delay")};
}

std::size_t get_uncompressed_size(const std_daq_protocol::ImageMetadata& data)
{
  return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
}

class EndTester
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit EndTester(sbc::RedisHandler& handler)
      : redis_handler(handler)
  {}

  uint64_t test_end_id()
  {
    using namespace std::chrono_literals;
    if (auto id = redis_handler.read_last_saved_image_id();
        id.has_value() && id.value() != last_saved_id)
    {
      last_saved_id = id.value() + 1;
      last_update = std::chrono::steady_clock::now();
    }
    if (10s < std::chrono::steady_clock::now() - last_update) std::exit(0);
    return last_saved_id;
  }

private:
  uint64_t last_saved_id = INVALID_IMAGE_ID;
  time_point last_update = std::chrono::steady_clock::now();
  sbc::RedisHandler& redis_handler;
};

} // namespace

int main(int argc, char* argv[])
{
  const auto args = read_arguments(argc, argv);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto sync_name = fmt::format("{}-image", args.config.detector_name);
  const std::size_t max_data_bytes = utils::converted_image_n_bytes(args.config);
  auto sender =
      cb::Communicator{{sync_name, max_data_bytes, utils::slots_number(args.config)},
                       {sync_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  sbc::RedisHandler redis_handler(args.config.detector_name, args.db_address, 1);
  sbc::BufferHandler reader(args.root_dir + args.config.detector_name, args.config.bit_depth / 8);
  std_daq_protocol::BufferedMetadata buffered_meta;
  EndTester end_id_tester(redis_handler);

  // zmq flags set to 0 to ensure that first data transfer for buffer reader is blocking
  // this is done to ensure that there is someone receiving the replayed stream
  auto zmq_flags = 0;

  while (true) {
    const auto end_id = std::min(args.end_image_id, end_id_tester.test_end_id());

    for (std::weakly_incrementable auto image : std::views::iota(args.start_image_id, end_id)) {
      if (redis_handler.receive(image, buffered_meta)) {
        if (auto size = get_uncompressed_size(buffered_meta.metadata()); size <= max_data_bytes) {
          reader.read(image, {sender.get_data(image), size}, buffered_meta.offset(),
                      buffered_meta.metadata().size());

          buffered_meta.mutable_metadata()->set_size(size);
          buffered_meta.mutable_metadata()->set_compression(std_daq_protocol::none);
          std::string meta_buffer_send;
          buffered_meta.metadata().SerializeToString(&meta_buffer_send);
          sender.send(image, meta_buffer_send, nullptr, zmq_flags);
          std::this_thread::sleep_for(std::chrono::milliseconds(args.delay));
          zmq_flags = ZMQ_NOBLOCK;
        }
      }
    }
  }
  return 0;
}
