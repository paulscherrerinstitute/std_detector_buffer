/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <span>
#include <cstdlib>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/gigafrost.hpp"
#include "utils/module_stats_collector.hpp"
#include "converter.hpp"

using namespace gf;

namespace {
void check_number_of_arguments(int argc)
{
  if (argc != 3) {
    fmt::print("Usage: std_data_convert_gf [detector_json_filename] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
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
  const uint16_t module_id = std::stoi(argv[2]);
  const auto quadrant = static_cast<quadrant_id>(module_id / 2);
  const auto converter_name = fmt::format("{}-{}-converted", config.detector_name, module_id);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto [module_bytes, converted_bytes] = calculate_data_sizes(config);

  auto ctx = zmq_ctx_new();

  utils::ModuleStatsCollector stats_collector("std_data_convert_gf", config.detector_name,
                                              module_id);
  auto receiver = cb::Communicator{{fmt::format("{}-{}", config.detector_name, module_id),
                                    module_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender = cb::Communicator{{sync_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  auto converter =
      sdc::Converter(config.image_pixel_height, config.image_pixel_width, quadrant, module_id);

  GFFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    stats_collector.processing_started();

    converter.convert(std::span<char>(image, module_bytes),
                      std::span<char>(sender.get_data(id), converted_bytes));

    sender.send(id, std::span((char*)(&meta), sizeof(meta)), nullptr);
    stats_collector.processing_finished();
  }
  return 0;
}
