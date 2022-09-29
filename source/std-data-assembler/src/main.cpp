/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/receiver.hpp"
#include "gigafrost.hpp"

namespace {
void check_number_of_arguments(int argc)
{
  if (argc != 2) {
    fmt::print("Usage: std_data_assembler [detector_json_filename] \n\n"
               "\tdetector_json_filename: detector config file path.\n");
    exit(-1);
  }
}
} // namespace

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));

  const auto PACKET_N_DATA_BYTES =
      gf::n_data_bytes_per_packet(config.image_pixel_height, config.image_pixel_width);
  const size_t FRAME_N_PACKETS =
      gf::n_packets_per_frame(config.image_pixel_height, config.image_pixel_width);

  auto ctx = zmq_ctx_new();
  // TODO[MP]: fixed module ID to be replaced
  auto receiver = cb::Receiver{
      {fmt::format("{}-{}", config.detector_name, 1), BYTES_PER_PACKET - PACKET_N_DATA_BYTES,
       PACKET_N_DATA_BYTES * FRAME_N_PACKETS, buffer_config::RAM_BUFFER_N_SLOTS},
      ctx};

  while (true) {
    auto [id, meta, image] = receiver.receive();
    (void)id;
  }
  return 0;
}