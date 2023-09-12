/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <span>
#include <cstdlib>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "detectors/gigafrost.hpp"
#include "utils/utils.hpp"
#include "converter.hpp"

using namespace gf;
using namespace buffer_config;

namespace {
std::tuple<utils::DetectorConfig, uint16_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_convert_gf");
  program.add_argument("module_id").scan<'d', uint16_t>();

  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get<uint16_t>("module_id")};
}

std::tuple<std::size_t, std::size_t> calculate_data_sizes(const utils::DetectorConfig& config)
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
  const auto [config, module_id] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_data_convert_gf", config.log_level};
  const auto quadrant = static_cast<quadrant_id>(module_id / 2);
  const auto converter_name = fmt::format("{}-{}-converted", config.detector_name, module_id);
  const auto [module_bytes, converted_bytes] = calculate_data_sizes(config);

  utils::stats::ModuleStatsCollector stats_collector(config.detector_name,
                                                     config.stats_collection_period, module_id);

  auto ctx = zmq_ctx_new();
  const auto source_name = fmt::format("{}-{}", config.detector_name, module_id);

  const cb::RamBufferConfig recv_buffer_config = {source_name, module_bytes, RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig recv_comm_config = {source_name, ctx, cb::CONN_TYPE_CONNECT,
                                                   ZMQ_SUB};
  auto receiver = cb::Communicator{recv_buffer_config, recv_comm_config};

  const auto sync_buffer_name = fmt::format("{}-image", config.detector_name);
  const auto sync_stream_name = fmt::format("{}-sync", config.detector_name);

  const cb::RamBufferConfig send_buffer_config = {sync_buffer_name, converted_bytes,
                                                  RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig send_comm_config = {sync_stream_name, ctx, cb::CONN_TYPE_CONNECT,
                                                   ZMQ_PUSH};
  auto sender = cb::Communicator{send_buffer_config, send_comm_config};

  auto converter =
      sdc::Converter(config.image_pixel_height, config.image_pixel_width, quadrant, module_id);

  GFFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    if (id != INVALID_IMAGE_ID) {
      utils::stats::process_stats p{stats_collector};
      converter.convert(std::span<char>(image, module_bytes),
                        std::span<char>(sender.get_data(id), converted_bytes));

      sender.send(id, std::span((char*)(&meta), sizeof(meta)), nullptr);
    }
    stats_collector.print_stats();
  }
  return 0;
}
