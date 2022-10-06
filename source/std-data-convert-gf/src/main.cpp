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

std::tuple<std::size_t, std::size_t> calculate_data_sizes(
    const buffer_utils::DetectorConfig& config)
{
  std::size_t MODULE_DATA_IN_BYTES =
      n_data_bytes_per_packet(config.image_pixel_height, config.image_pixel_width) *
          (n_packets_per_frame(config.image_pixel_height, config.image_pixel_width) - 1) +
      last_packet_n_bytes(config.image_pixel_height, config.image_pixel_width);
  std::size_t CONVERTED_DATA_SIZE =
      converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  return {MODULE_DATA_IN_BYTES, CONVERTED_DATA_SIZE};
}

} // namespace

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  const uint16_t module_id = std::stoi(argv[3]);
  const auto converter_name = fmt::format("{}-{}-converted", config.detector_name, module_id);
  const auto [module_bytes, converted_bytes] = calculate_data_sizes(config);

  auto ctx = zmq_ctx_new();

  sdc::StatsCollector stats_collector(converter_name, std::stoi(argv[2]));
  auto receiver = cb::Receiver{{fmt::format("{}-{}", config.detector_name, module_id),
                                sizeof(GFFrame), module_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                               ctx};

  auto sender = cb::Sender{
      {converter_name, sizeof(GFFrame), converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS}, ctx};

  while (true) {
    auto [id, meta, image] = receiver.receive();
    stats_collector.processing_started();
    sender.send(id, meta, image);
    stats_collector.processing_finished();
  }
  return 0;
}
