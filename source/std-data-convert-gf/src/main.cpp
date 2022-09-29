/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/receiver.hpp"
#include "core_buffer/sender.hpp"
#include "gigafrost.hpp"

using namespace gf;

namespace {
void check_number_of_arguments(int argc)
{
  if (argc != 4) {
    fmt::print("Usage: std_data_convert_gf [detector_json_filename] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tquadrant_id: Supported quadrant_id\n"
               "\tmodule_id: module id - data source\n");
    exit(-1);
  }
}
} // namespace

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  const uint16_t module_id = std::stoi(argv[3]);

  const auto ORIGINAL_PACKET_N_DATA_BYTES =
      n_data_bytes_per_packet(config.image_pixel_height, config.image_pixel_width);
  const auto FRAME_N_PACKETS =
      n_packets_per_frame(config.image_pixel_height, config.image_pixel_width);
  const auto HEADER_SIZE = BYTES_PER_PACKET - ORIGINAL_PACKET_N_DATA_BYTES;
  const auto CONVERTED_PACKET_N_DATA_BYTES = ORIGINAL_PACKET_N_DATA_BYTES / 3 * 4;

  auto ctx = zmq_ctx_new();

  auto receiver = cb::Receiver{{fmt::format("{}-{}", config.detector_name, module_id), HEADER_SIZE,
                                ORIGINAL_PACKET_N_DATA_BYTES * FRAME_N_PACKETS,
                                buffer_config::RAM_BUFFER_N_SLOTS},
                               ctx};

  auto sender = cb::Sender{{fmt::format("{}-converted", config.detector_name, module_id),
                            HEADER_SIZE, CONVERTED_PACKET_N_DATA_BYTES * FRAME_N_PACKETS,
                            buffer_config::RAM_BUFFER_N_SLOTS},
                           ctx};

  while (true) {
    auto [id, meta, image] = receiver.receive();
    sender.send(id, nullptr, image);
  }
  return 0;
}
