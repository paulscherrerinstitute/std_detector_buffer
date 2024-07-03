/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <string>

#include <fmt/core.h>
#include <zmq.h>

#include "utils/utils.hpp"
#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/writer_action.pb.h"
#include "writer_stats_collector.hpp"
#include "hdf5_file.hpp"

using namespace std;

int main(int argc, char* argv[])
{
  const std::string program_name{"std_det_writer"};
  auto program = utils::create_parser(program_name);

  program->add_argument("writer_id").scan<'d', uint16_t>();
  program->add_argument("-s", "--source_suffix")
      .help("suffix for shared memory source for ram_buffer - default \"image\"")
      .default_value("image"s);

  program = utils::parse_arguments(std::move(program), argc, argv);

  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  const auto writer_id = program->get<uint16_t>("writer_id");
  [[maybe_unused]] utils::log::logger l{program_name, config.log_level};

  if (config.number_of_writers <= writer_id) return 0; // shutdown - writer not needed

  const auto suffix = program->get("--source_suffix");
  const size_t image_n_bytes = utils::converted_image_n_bytes(config);

  std::unique_ptr<HDF5File> file;
  WriterStatsCollector stats(config.detector_name, suffix, config.stats_collection_period,
                             image_n_bytes, writer_id);

  const auto buffer_name = fmt::format("{}-{}", config.detector_name, suffix);
  const auto source_name =
      fmt::format("{}-driver-{}", config.detector_name, program->get<uint16_t>("writer_id"));
  const auto sink_name =
      fmt::format("{}-writer-{}", config.detector_name, program->get<uint16_t>("writer_id"));

  auto ctx = zmq_ctx_new();
  auto receiver = cb::Communicator{{buffer_name, image_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PULL}};

  auto sender = buffer_utils::bind_socket(ctx, sink_name, ZMQ_PUSH);

  char buffer[512];
  std_daq_protocol::WriterAction action;
  std_daq_protocol::WriterResponse response;
  response.set_code(std_daq_protocol::ResponseCode::SUCCESS);
  std::string send_msg;
  response.SerializeToString(&send_msg);

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      action.ParseFromArray(buffer, n_bytes);

      if (action.has_create_file()) {
        const auto& create_file = action.create_file();
        file = std::make_unique<HDF5File>(config, create_file.path(), suffix);
        zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
      }
      else if (action.has_record_image()) {
        const auto& record_image = action.record_image();
        auto image_data = receiver.get_data(record_image.image_metadata().image_id());
        stats.start_image_write();
        file->write(record_image.image_metadata(), image_data);
        stats.end_image_write();
      }
      else if (action.has_close_file()) {
        file.reset();
        zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
      }
      else if (action.has_confirm_last_image())
        zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
    }
    stats.print_stats();
  }
  return 0;
}
