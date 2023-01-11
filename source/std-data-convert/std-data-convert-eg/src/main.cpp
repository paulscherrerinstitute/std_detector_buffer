/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <span>

#include <argparse/argparse.hpp>
#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/eiger.hpp"
#include "utils/module_stats_collector.hpp"
#include "utils/args.hpp"
#include "converter.hpp"

namespace {
std::tuple<buffer_utils::DetectorConfig, uint16_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_convert_eg");
  program.add_argument("module_id").scan<'d', uint16_t>();

  program = utils::parse_arguments(program, argc, argv);

  return {buffer_utils::read_json_config(program.get("detector_json_filename")),
          program.get<uint16_t>("module_id")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, module_id] = read_arguments(argc, argv);
  if (config.bit_depth < 8) throw std::runtime_error("Bit depth below 8 is not supported!");

  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const size_t frame_n_bytes = MODULE_N_PIXELS * config.bit_depth / 8;
  const size_t converted_bytes = eg::converted_image_n_bytes(
      config.image_pixel_height, config.image_pixel_width, config.bit_depth);

  auto ctx = zmq_ctx_new();

  utils::ModuleStatsCollector stats_collector("std_data_convert_eg", config.detector_name,
                                              module_id);

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
