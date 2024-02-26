/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <ranges>

#include <zmq.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/writer_command.pb.h"
#include "utils/utils.hpp"

using namespace std::string_literals;

namespace {

std::tuple<utils::DetectorConfig, std::string, std::size_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_det_driver");
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer")
      .default_value("image"s);
  program->add_argument("-n", "--number_of_writers")
      .default_value(4ul)
      .scan<'u', std::size_t>()
      .help("Number of parallel writers");

  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("--source_suffix"), program->get<std::size_t>("--number_of_writers")};
}

void* bind_sender_socket(void* ctx, const std::string& stream_address)
{
  constexpr auto zmq_sndhwm = 100;

  void* socket = zmq_socket(ctx, ZMQ_PUSH);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));
  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

constexpr auto zmq_io_threads = 4;

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, source_suffix, number_of_writers] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_det_driver", config.log_level};
  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-{}", config.detector_name, source_suffix);

  auto receiver = cb::Communicator{
      {source_name, sizeof(std_daq_protocol::ImageMetadata), buffer_config::RAM_BUFFER_N_SLOTS},
      {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  std::vector<void*> sender_sockets;
  std::ranges::for_each(std::views::iota(0u, number_of_writers), [&](auto i) {
    sender_sockets.push_back(
        bind_sender_socket(ctx, fmt::format("{}-driver-{}", config.detector_name, i)));
  });

  std_daq_protocol::WriterAction msg;
  auto createFileCmd = msg.mutable_create_file();
  createFileCmd->set_path("/gpfs/test/test-beamline/file0.h5");
  createFileCmd->set_writer_id(0);

  std::string cmd;
  msg.SerializeToString(&cmd);
  zmq_send(sender_sockets[0], cmd.c_str(), cmd.size(), 0);

  createFileCmd->set_path("/gpfs/test/test-beamline/file1.h5");
  msg.SerializeToString(&cmd);
  zmq_send(sender_sockets[1], cmd.c_str(), cmd.size(), 0);

  auto i = 0;
  for(auto ii = 0; ii < 100; ii++) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      memcpy(receiver.get_data(meta.image_id()), buffer, n_bytes);

      std_daq_protocol::WriterAction action;
      action.mutable_record_image()->set_allocated_image_metadata(&meta);
      msg.SerializeToString(&cmd);
      zmq_send(sender_sockets[i], cmd.c_str(), cmd.size(), 0);
      i = (i + 1) % 2;
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
