/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <thread>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer_common/redis_handler.hpp"
#include "std_buffer_common/file_handler.hpp"
#include "std_buffer/buffered_metadata.pb.h"
#include "std_buffer/image_buffer.pb.h"
#include "utils/utils.hpp"

namespace {

constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;

struct arguments
{
  utils::DetectorConfig config;
  std::string root_dir;
  std::string db_address;
};

arguments read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_buffer_reader");
  program->add_argument("--root_dir").help("Root directory where files will be stored").required();
  program->add_argument("--db_address")
      .help("Address of Redis API compatible database to connect")
      .required();

  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--root_dir"), program->get("--db_address")};
}

void* bind_driver_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_REP);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

// std::size_t get_uncompressed_size(const std_daq_protocol::ImageMetadata& data)
// {
//   return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
// }

} // namespace

int main(int argc, char* argv[])
{
  const auto args = read_arguments(argc, argv);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto sync_name = fmt::format("{}-image", args.config.detector_name);
  const std::size_t max_data_bytes = utils::converted_image_n_bytes(args.config);
  auto sender = cb::Communicator{{sync_name, max_data_bytes, utils::slots_number(args.config)},
                                 {sync_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  auto driver_address =
      fmt::format("{}{}-driver", buffer_config::IPC_URL_BASE, args.config.detector_name);
  void* driver_socket = bind_driver_socket(ctx, driver_address);

  sbc::RedisHandler redis_handler(args.config.detector_name, args.db_address, 1);
  sbc::FileHandler reader(args.root_dir + args.config.detector_name, args.config.bit_depth / 8);
  std_daq_protocol::BufferedMetadata buffered_meta;

  std_daq_protocol::RequestNextImage request;
  std_daq_protocol::NextImageResponse response;
  std::string cmd;
  char buffer[512];

  while (true) {
    if (const auto n_bytes = zmq_recv(driver_socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
      request.ParseFromArray(buffer, n_bytes);
      (void) request.image_id();
    }

  }

  //  while (true) {
  //   const auto end_id = std::min(args.end_image_id, end_id_tester.test_end_id());
  //
  //   for (std::weakly_incrementable auto image : std::views::iota(args.start_image_id, end_id)) {
  //     if (redis_handler.receive(image, buffered_meta)) {
  //       if (auto size = get_uncompressed_size(buffered_meta.metadata()); size <= max_data_bytes)
  //       {
  //         reader.read(image, {sender.get_data(image), size}, buffered_meta.offset(),
  //                     buffered_meta.metadata().size());
  //
  //         buffered_meta.mutable_metadata()->set_size(size);
  //         buffered_meta.mutable_metadata()->set_compression(std_daq_protocol::none);
  //         std::string meta_buffer_send;
  //         buffered_meta.metadata().SerializeToString(&meta_buffer_send);
  //         sender.send(image, meta_buffer_send, nullptr, zmq_flags);
  //         std::this_thread::sleep_for(std::chrono::milliseconds(args.delay));
  //         zmq_flags = ZMQ_NOBLOCK;
  //       }
  //     }
  //   }
  // }
  return 0;
}
