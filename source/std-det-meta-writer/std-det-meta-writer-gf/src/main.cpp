/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <string>

#include <fmt/core.h>
#include <zmq.h>

#include "utils/utils.hpp"
#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/writer_action.pb.h"
#include "hdf5_file.hpp"

#include <core_buffer/buffer_utils.hpp>

using namespace std;

namespace {
constexpr uid_t root_user_id = 0;

bool switch_user(uid_t new_user_id)
{
  return seteuid(new_user_id) != -1;
}

bool switch_writer_user(uid_t writer_user)
{
  return writer_user == root_user_id || switch_user(writer_user);
}

} // namespace

int main(int argc, char* argv[])
{
  const std::string program_name{"std_det_meta_writer_gf"};
  auto program = utils::create_parser(program_name);

  program = utils::parse_arguments(std::move(program), argc, argv);
  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{program_name, config.log_level};

  std::unique_ptr<HDF5File> file;
  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);

  const auto source_name = fmt::format("{}-driver-meta", config.detector_name);
  const auto sink_name = fmt::format("{}-writer-meta", config.detector_name);

  auto ctx = zmq_ctx_new();

  auto receiver = cb::Communicator{
      {source_name, utils::converted_image_n_bytes(config), utils::slots_number(config)},
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
        if (const auto& create_file = action.create_file();
            switch_writer_user(create_file.writer_id()))
        {
          file = std::make_unique<HDF5File>(config, create_file.path());
          zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
        }
      }
      else if (action.has_record_image()) {
        const auto& record_image = action.record_image();
        file->write(record_image.image_metadata());
        stats.process();
      }
      else if (action.has_close_file()) {
        file.reset();
        switch_user(root_user_id);
        zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
      }
      else if (action.has_confirm_last_image())
        zmq_send(sender, send_msg.c_str(), send_msg.size(), 0);
    }
    stats.print_stats();
  }
  return 0;
}
