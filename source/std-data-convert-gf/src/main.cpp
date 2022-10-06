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
#include "stats_collector.hpp"

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
  const auto converter_name = fmt::format("{}-{}-converted", config.detector_name, module_id);
  sdc::StatsCollector stats_collector(converter_name, std::stoi(argv[2]));

  const auto ORIGINAL_DATA_N_BYTES_FRAME =
      n_data_bytes_per_packet(config.image_pixel_height, config.image_pixel_width) *
          (n_packets_per_frame(config.image_pixel_height, config.image_pixel_width) - 1) +
      last_packet_n_bytes(config.image_pixel_height, config.image_pixel_width);
  const auto CONVERTED_DATA_N_BYTES_FRAME = ORIGINAL_DATA_N_BYTES_FRAME / 3 * 4;

  auto ctx = zmq_ctx_new();

  auto receiver =
      cb::Receiver{{fmt::format("{}-{}", config.detector_name, module_id), sizeof(GFFrame),
                    ORIGINAL_DATA_N_BYTES_FRAME, buffer_config::RAM_BUFFER_N_SLOTS},
                   ctx};

  auto sender = cb::Sender{{converter_name, sizeof(GFFrame), CONVERTED_DATA_N_BYTES_FRAME,
                            buffer_config::RAM_BUFFER_N_SLOTS},
                           ctx};

  while (true) {
    auto [id, meta, image] = receiver.receive();
    stats_collector.processing_started();
    fmt::print("WOAH {} {} {} {} {} {}", (int)image[0], (int)image[1], (int)image[2], (int)image[3],
               (int)image[4], (int)image[5]);
    sender.send(id, meta, image);
    stats_collector.processing_finished();
  }
  return 0;
}
