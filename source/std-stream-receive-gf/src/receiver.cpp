/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "gigafrost.hpp"
#include "ram_buffer.hpp"

std::tuple<buffer_utils::DetectorConfig, std::size_t, std::size_t> read_arguments(int argc, char* argv[])
{
  if (argc != 4) {
    fmt::print("Usage: std_stream_receive_gf [detector_json_filename]"
               " [stream_address_first_half] [stream_address_second_half] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tstream_address_first_half: address to bind the input stream"
               " - first half of image.\n"
               "\tstream_address_second_half:address to bind the input stream"
               " - first half of image.\n");
    exit(-1);
  }
  return {buffer_utils::read_json_config(argv[1]), std::stoi(argv[2]), std::stoi(argv[3])};
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, is_first_half] = read_arguments(argc, argv);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto converted_bytes =
      gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto data_bytes_sent = converted_bytes / 2;

  return data_bytes_sent;
}
