/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <fmt/core.h>
#include <zmq.h>

#include "utils/utils.hpp"
#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/writer_command.pb.h"
#include "writer_stats_collector.hpp"
#include "hdf5_file.hpp"

using namespace std;

void* create_socket(void* ctx, const std::string& ipc_address, const int zmq_socket_type)
{

  void* socket = zmq_socket(ctx, zmq_socket_type);
  if (socket == nullptr)
    throw runtime_error(std::string("Cannot create socket: ") + zmq_strerror(errno));

  if (zmq_socket_type == ZMQ_SUB && zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0)
    throw runtime_error(std::string("Cannot subscribe socket: ") + zmq_strerror(errno));

  // SUB and PULL sockets are used for receiving, the rest for sending.
  if (zmq_socket_type == ZMQ_SUB || zmq_socket_type == ZMQ_PULL) {
    int rcvhwm = 0;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0)
      throw runtime_error(std::string("Cannot set RCVHWM: ") + zmq_strerror(errno));

    const int timeout = 1000;
    if (zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
      throw runtime_error(std::string("Cannot set timeout: ") + zmq_strerror(errno));
  }
  else {
    const int sndhwm = 10000;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
      throw runtime_error(std::string("Cannot set SNDHWM:") + zmq_strerror(errno));
  }

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw runtime_error(std::string("Cannot set linger: ") + zmq_strerror(errno));

  const auto ipc = std::string("ipc:///tmp/") + ipc_address;
  if (zmq_connect(socket, ipc.c_str()) != 0) throw runtime_error(zmq_strerror(errno));

  return socket;
}

int main(int argc, char* argv[])
{
  const std::string program_name{"std_det_writer"};
  auto program = utils::create_parser(program_name);

  program->add_argument("writer_id").scan<'d', uint16_t>();
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer - default \"image\"")
      .default_value("image"s);

  program = utils::parse_arguments(std::move(program), argc, argv);

  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{program_name, config.log_level};
  const auto suffix = program->get("--source_suffix");
  const size_t image_n_bytes = utils::converted_image_n_bytes(config);

  std::unique_ptr<HDF5File> file;
  WriterStatsCollector stats(config.detector_name, suffix, config.stats_collection_period,
                             image_n_bytes);

  const auto buffer_name = fmt::format("{}-{}", config.detector_name, suffix);
  const auto source_name =
      fmt::format("{}-driver-{}", config.detector_name, program->get<uint16_t>("writer_id"));

  auto ctx = zmq_ctx_new();
  auto receiver = cb::Communicator{{buffer_name, image_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PULL}};

  char buffer[512];
  std_daq_protocol::WriterAction msg;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      msg.ParseFromArray(buffer, n_bytes);

      if (msg.has_create_file()) {
        const auto& create_file = msg.create_file();
        file = std::make_unique<HDF5File>(config, create_file.path(), suffix);
      }
      else if (msg.has_record_image()) {
        const auto& record_image = msg.record_image();
        auto image_data = receiver.get_data(record_image.image_metadata().image_id());
        stats.start_image_write();
        file->write({image_data, image_n_bytes}, record_image.image_metadata());
        stats.end_image_write();
      }
      else
        file.reset();
    }
    stats.print_stats();
  }
  return 0;
}
