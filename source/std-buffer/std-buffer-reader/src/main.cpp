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
#include "std_buffer/image_buffer.pb.h"
#include "utils/utils.hpp"

#include "buffer_handler.hpp"

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

  const int timeout_ms = 1000;
  zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));

  return socket;
}

} // namespace

int main(int argc, char* argv[])
{
  const auto args = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_buffer_reader", args.config.log_level};

  utils::stats::TimedStatsCollector stats(args.config.detector_name,
                                          args.config.stats_collection_period);

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
  auto buffer_handler = std::make_unique<BufferHandler>(redis_handler, args.root_dir,
                                                        args.config.bit_depth / 8, sender);

  std_daq_protocol::RequestNextImage request;
  char buffer[512];

  buffer_handler->start_loader();
  while (true) {
    if (const auto n_bytes = zmq_recv(driver_socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
      request.ParseFromArray(buffer, n_bytes);
      if (request.new_request()) {
        buffer_handler = std::make_unique<BufferHandler>(redis_handler, args.root_dir,
                                                         args.config.bit_depth / 8, sender);
        buffer_handler->start_loader();
      }

      std_daq_protocol::NextImageResponse response;
      std::string cmd;

      if (auto image = buffer_handler->get_image(request.image_id())) {
        image->SerializeToString(&cmd);
        sender.send(image->image_id(), cmd, nullptr, 0);
        response.mutable_ack()->set_image_id(image->image_id());
      }
      else
        response.mutable_no_image();

      response.SerializeToString(&cmd);
      zmq_send(driver_socket, cmd.c_str(), cmd.length(), 0);
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
