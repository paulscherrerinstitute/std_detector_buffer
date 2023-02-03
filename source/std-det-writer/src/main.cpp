/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <zmq.h>
#include <mpi.h>
#include <rapidjson/document.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "utils/args.hpp"

#include "live_writer_config.hpp"
#include "WriterStats.hpp"
#include "JFH5Writer.hpp"
#include "DetWriterConfig.hpp"
#include "core_buffer/ram_buffer.hpp"

#include "std_daq/writer_command.pb.h"

using namespace std;
using namespace live_writer_config;

int main(int argc, char* argv[])
{
  auto program = utils::create_parser("std_det_writer");
  program = utils::parse_arguments(program, argc, argv);
  const auto config = converter::from_json_file(program.get("detector_json_filename"));
  const size_t image_n_bytes = config.image_width * config.image_height * config.bit_depth / 8;

  MPI_Init(nullptr, nullptr);
  int n_writers;
  MPI_Comm_size(MPI_COMM_WORLD, &n_writers);
  int i_writer;
  MPI_Comm_rank(MPI_COMM_WORLD, &i_writer);

  JFH5Writer writer(config.detector_name);
  WriterStats stats(config.detector_name, image_n_bytes);

  const auto buffer_name = fmt::format("{}-image", config.detector_name);
  RamBuffer image_buffer(buffer_name, image_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS);
  auto ctx = zmq_ctx_new();
  const auto command_stream_name = fmt::format("{}-writer", config.detector_name);
  auto command_receiver = buffer_utils::connect_socket(ctx, command_stream_name, ZMQ_SUB);

  char recv_buffer_meta[512];
  bool open_run = false;
  std_daq_protocol::WriterCommand command;

  while (true) {
    auto nbytes = zmq_recv(command_receiver, &recv_buffer_meta, sizeof(recv_buffer_meta), 0);
    if (nbytes == -1) continue;

    command.ParseFromString(string(recv_buffer_meta));

    // Handle start and stop commands.
    switch (command.command_type()) {
        case std_daq_protocol::CommandType::START_WRITING:
          writer.open_run(command.run_info().output_file(), command.run_info().run_id(), command.run_info().n_images(), 
                          config.image_height, config.image_width, config.bit_depth);
          open_run = true;
          continue;
          
        case std_daq_protocol::CommandType::STOP_WRITING:
          writer.close_run();
          stats.end_run();
          open_run = false;
          continue;

        default:
          break;
    }

    if (open_run) throw std::runtime_error("Unexpected protocol message. Send START_WRITING before WRITE_IMAGE.");

    const auto i_image = command.i_image();
    const auto image_id = command.metadata().image_id();
    const auto run_id = command.run_info().run_id();
    const auto data = image_buffer.get_data(image_id);

    // Fair distribution of images among writers.
    if (i_image % n_writers == (uint) i_writer) {
      stats.start_image_write();
      writer.write_data(run_id, i_image, data);
      stats.end_image_write();
    }

    // Only the first instance writes metadata.
    if (i_writer == 0) {
      writer.write_meta(run_id, i_image, command.metadata());
    }
  }
}
