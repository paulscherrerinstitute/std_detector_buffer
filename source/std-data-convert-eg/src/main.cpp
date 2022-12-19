/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <span>
#include <cstdlib>

#include <zmq.h>
#include <fmt/core.h>

#include "buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "eiger.hpp"
#include "stats_collector.hpp"
#include "converter.hpp"

namespace {
void check_number_of_arguments(int argc)
{
  if (argc != 3) {
    fmt::print("Usage: std_data_convert_eg [detector_json_filename] \n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tmodule_id: module id - data source\n");
    exit(-1);
  }
}

} // namespace

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  const uint16_t module_id = std::stoi(argv[2]);
  const auto converter_name = fmt::format("{}-{}-converted", config.detector_name, module_id);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const size_t frame_n_bytes = MODULE_N_PIXELS * config.bit_depth / 8;
  const size_t converted_bytes = eg::converted_image_n_bytes(
      config.image_pixel_height, config.image_pixel_width, config.bit_depth);

  auto ctx = zmq_ctx_new();

  eg::sdc::StatsCollector stats_collector(converter_name, module_id);

  auto receiver = cb::Communicator{{fmt::format("{}-{}", config.detector_name, module_id),
                                    frame_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender = cb::Communicator{{sync_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  auto converter = eg::sdc::Converter(config.image_pixel_height, config.image_pixel_width,
                                      config.bit_depth, module_id);

  EGFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));

    converter.convert(std::span<char>(image, frame_n_bytes),
                      std::span<char>(sender.get_data(id), converted_bytes));

    sender.send(id, std::span((char*)(&meta), sizeof(meta)), nullptr);
  }
  return 0;
}
