/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <span>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "detectors/eiger.hpp"
#include "utils/utils.hpp"
#include "converter.hpp"

using namespace buffer_config;

namespace {
std::tuple<utils::DetectorConfig, uint16_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_convert_eg");
  program.add_argument("module_id").scan<'d', uint16_t>();

  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get<uint16_t>("module_id")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, module_id] = read_arguments(argc, argv);
  if (config.bit_depth < 8) throw std::runtime_error("Bit depth below 8 is not supported!");
  [[maybe_unused]] utils::log::logger l{"std_data_convert_eg", config.log_level};

  const size_t frame_n_bytes = MODULE_N_PIXELS * config.bit_depth / 8;
  const size_t converted_bytes = eg::converted_image_n_bytes(
      config.image_pixel_height, config.image_pixel_width, config.bit_depth);

  utils::stats::ModuleStatsCollector stats_collector(config.detector_name, module_id);

  auto ctx = zmq_ctx_new();
  const auto source_name = fmt::format("{}-{}", config.detector_name, module_id);

  const cb::RamBufferConfig recv_buffer_config = {source_name, frame_n_bytes, RAM_BUFFER_N_SLOTS};
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

  auto converter = eg::sdc::Converter(config, module_id);

  EGFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));
    if (id != INVALID_IMAGE_ID) {
      utils::stats::process_stats p{stats_collector};
      converter.convert(std::span<char>(image, frame_n_bytes),
                        std::span<char>(sender.get_data(id), converted_bytes));

      sender.send(id, std::span((char*)(&meta), sizeof(meta)), nullptr);
    }
    stats_collector.print_stats();
  }
  return 0;
}
