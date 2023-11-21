/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/jungfrau.hpp"
#include "utils/utils.hpp"

#include "converter.hpp"
#include "read_gains_and_pedestals.hpp"

auto read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_convert_jf");
  program.add_argument("gains_and_pedestal_h5_filename");
  program.add_argument("module_id").scan<'d', uint16_t>();
  return utils::parse_arguments(program, argc, argv);
}

jf::sdc::Converter create_converter(const std::string& filename,
                                    const utils::DetectorConfig& config,
                                    int module_id)
{
  const auto [gains, pedestals] = jf::sdc::read_gains_and_pedestals(
      filename, config.image_pixel_height * config.image_pixel_width);
  return jf::sdc::Converter{gains, pedestals, config, module_id};
}

int main(int argc, char* argv[])
{
  auto parser = read_arguments(argc, argv);

  const auto config = utils::read_config_from_json_file(parser.get("detector_json_filename"));
  [[maybe_unused]] utils::log::logger l{"std_data_convert_jf", config.log_level};
  const uint16_t module_id = parser.get<uint16_t>("module_id");
  utils::stats::ModuleStatsCollector stats_collector(config.detector_name,
                                                     config.stats_collection_period, module_id);

  auto converter =
      create_converter(parser.get("gains_and_pedestal_h5_filename"), config, module_id);

  auto ctx = zmq_ctx_new();

  const size_t frame_n_bytes = MODULE_N_PIXELS * config.bit_depth / 8;
  const auto source_name = fmt::format("{}-{}", config.detector_name, module_id);
  auto receiver = cb::Communicator{{source_name, frame_n_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                   {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  const size_t converted_bytes = utils::converted_image_n_bytes(config);
  const size_t converted_floats = converted_bytes / sizeof(float);
  const auto sync_buffer_name = fmt::format("{}-image", config.detector_name);
  const auto sync_stream_name = fmt::format("{}-sync", config.detector_name);
  auto sender =
      cb::Communicator{{sync_buffer_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                       {sync_stream_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  JFFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    if (id != INVALID_IMAGE_ID) {
      auto data = sender.get_data(id);
      converter.convert_data({(uint16_t*)image, MODULE_N_PIXELS}, {(float*)data, converted_floats});
      sender.send(id, std::span<char>((char*)&meta, sizeof(meta)), nullptr);
      stats_collector.process();
    }
    stats_collector.print_stats();
  }
  return 0;
}
