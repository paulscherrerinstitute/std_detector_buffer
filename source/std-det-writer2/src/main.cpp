/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <string>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <zmq.h>

#include "utils/utils.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/writer_command.pb.h"
#include "writer_stats_collector.hpp"
#include "H5Writer.hpp"

using namespace std;

void* create_socket(void* ctx, const std::string& ipc_address, const int zmq_socket_type)
{

  void* socket = zmq_socket(ctx, zmq_socket_type);
  if (socket == nullptr)
    throw runtime_error(std::string("Cannot create socket: ") + zmq_strerror(errno));

  if (zmq_socket_type == ZMQ_SUB && zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0)
    throw runtime_error(std::string("Cannot subscrive socket: ") + zmq_strerror(errno));

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
  program = utils::parse_arguments(std::move(program), argc, argv);
  const auto config = utils::read_config_from_json_file(program->get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{program_name, config.log_level};
  const size_t image_n_bytes =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;

  //  MPI_Init(nullptr, nullptr);
  int n_writers{1};
  int i_writer{0};
  //  MPI_Comm_size(MPI_COMM_WORLD, &n_writers);

  H5Writer writer(config.detector_name);
  WriterStatsCollector stats(config.detector_name, config.stats_collection_period, image_n_bytes);
  const auto buffer_name = fmt::format("{}-blosc2", config.detector_name);
  RamBuffer image_buffer(buffer_name, image_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS);
  auto ctx = zmq_ctx_new();
  const auto command_stream_name = fmt::format("{}-writer", config.detector_name);
  auto command_receiver = create_socket(ctx, command_stream_name, ZMQ_SUB);

  const auto status_sender_name = fmt::format("{}-writer-status-sync", config.detector_name);
  auto status_sender = create_socket(ctx, status_sender_name, ZMQ_PUSH);

  char recv_buffer_meta[512];
  uint64_t current_run_id = -1;
  uint32_t highest_run_image_index = 0;

  std_daq_protocol::WriterCommand command;
  std_daq_protocol::WriterStatus status;

  std::string status_buffer_send;
  status.set_i_writer(i_writer);
  status.set_n_writers(n_writers);

  const auto process_exception = [&](std::string message) {
    status.set_command_type(std_daq_protocol::CommandType::STOP_WRITING);
    status.set_error_message("ERROR: " + message);
    status.SerializeToString(&status_buffer_send);
    zmq_send(status_sender, status_buffer_send.c_str(), status_buffer_send.size(), 0);
  };

  if (config.writer_user_id > 0) {
    if (seteuid(config.writer_user_id) == -1) {
      throw runtime_error("Cannot set uid=" + std::to_string(config.writer_user_id));
    }
  }

  while (true) {
    stats.print_stats();

    auto nbytes = zmq_recv(command_receiver, &recv_buffer_meta, sizeof(recv_buffer_meta), 0);
    if (nbytes == -1) continue;

    command.ParseFromString(string(recv_buffer_meta));

    const auto run_id = command.run_info().run_id();
    const auto output_file = command.run_info().output_file();
    const auto n_images = command.run_info().n_images();

    const auto image_id = command.metadata().image_id();
    const auto i_image = command.i_image();
    const auto data = image_buffer.get_data(image_id);
    auto data_size = command.metadata().size();

    // Handle start and stop commands.
    switch (command.command_type()) {
    case std_daq_protocol::CommandType::START_WRITING:
      spdlog::info("Start writing run_id={} output_file={} n_images={}", run_id, output_file,
                   n_images);
      try {
        writer.open_run(output_file, run_id, n_images, config.image_pixel_height,
                        config.image_pixel_width, config.bit_depth);
      }
      catch (const std::exception& ex) {
        process_exception(ex.what());
        continue;
      }

      current_run_id = run_id;
      highest_run_image_index = 0;

      status.set_command_type(std_daq_protocol::CommandType::START_WRITING);
      status.set_i_image(0);
      status.set_allocated_run_info(command.release_run_info());
      status.set_error_message("");
      status.SerializeToString(&status_buffer_send);
      zmq_send(status_sender, status_buffer_send.c_str(), status_buffer_send.size(), 0);

      continue;

    case std_daq_protocol::CommandType::STOP_WRITING:
      spdlog::info("Stop writing run_id={} output_file={} last_i_image={}", run_id, output_file,
                   i_image);
      try {
        writer.close_run(highest_run_image_index);

        if ((uint32_t)n_images != highest_run_image_index + 1) {
          status.set_error_message("Interrupted.");
        }
        else {
          status.set_error_message("Completed.");
        }

        status.set_command_type(std_daq_protocol::CommandType::STOP_WRITING);
        status.set_i_image(0);
        status.SerializeToString(&status_buffer_send);
        zmq_send(status_sender, status_buffer_send.c_str(), status_buffer_send.size(), 0);
      }
      catch (const std::exception& ex) {
        process_exception(ex.what());
      }
      current_run_id = -1;
      continue;

    default:
      if (current_run_id == -1LU) continue;
    }

    // Check if we got a message for the wrong run_id - should not happen, driver problem.
    if (run_id != current_run_id) {
      spdlog::error("Received write request for run_id={} but current_run_id={}", run_id,
                    current_run_id);
      continue;
    }

    highest_run_image_index = max(highest_run_image_index, i_image);

    // Fair distribution of images among writers.
    if (i_image % n_writers == (uint)i_writer) {
      spdlog::debug("i_writer={} i_image={} image_id={} run_id={}", i_writer, i_image, image_id,
                    run_id);

      stats.start_image_write();
      writer.write_data(run_id, i_image, data_size, data);
      stats.end_image_write();

      status.set_command_type(std_daq_protocol::CommandType::WRITE_IMAGE);
      status.set_i_image(i_image);
      status.SerializeToString(&status_buffer_send);
      zmq_send(status_sender, status_buffer_send.c_str(), status_buffer_send.size(), 0);
    }

    // Only the first instance writes metadata.
    if (i_writer == 0) {
      writer.write_meta(run_id, i_image, command.metadata());
    }
  }
}
