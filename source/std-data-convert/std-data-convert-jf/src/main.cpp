/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/jungfrau.hpp"
#include "utils/module_stats_collector.hpp"
#include "utils/args.hpp"

#include "identifier.hpp"
#include "converter.hpp"
#include "read_gains_and_pedestals.hpp"

cb::Communicator create_receiver(std::string name, void* ctx)
{
  return cb::Communicator{{std::move(name), DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME,
                           buffer_config::RAM_BUFFER_N_SLOTS},
                          {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
}

cb::Communicator create_sender(std::string name, void* ctx)
{
  return cb::Communicator{
      {std::move(name), MODULE_N_PIXELS * sizeof(float), buffer_config::RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};
}

auto read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_convert_jf");
  program.add_argument("gains_and_pedestal_h5_filename");
  program.add_argument("module_id").scan<'d', uint16_t>();
  program.add_argument("converter_index").scan<'d', uint16_t>();
  return utils::parse_arguments(program, argc, argv);
}

jf::sdc::Converter create_converter(const std::string& filename, std::size_t image_size)
{
  const auto [gains, pedestals] = jf::sdc::read_gains_and_pedestals(filename, image_size);
  return jf::sdc::Converter{gains, pedestals};
}

int main(int argc, char* argv[])
{
  auto parser = read_arguments(argc, argv);

  const auto config = buffer_utils::read_json_config(parser.get("detector_json_filename"));
  const uint16_t module_id = parser.get<uint16_t>("module_id");
  const uint16_t converter_index = parser.get<uint16_t>("converter_index");
  const jf::sdc::Identifier converter_id(config.detector_name, module_id, converter_index);
  utils::ModuleStatsCollector stats_collector("std_data_convert_jf", config.detector_name,
                                              module_id);

  auto converter = create_converter(parser.get("gains_and_pedestal_h5_filename"),
                                    config.image_pixel_height * config.image_pixel_width);

  auto ctx = zmq_ctx_new();
  auto receiver = create_receiver(converter_id.source_name(), ctx);
  auto sender = create_sender(converter_id.converter_name(), ctx);

  JFFrame meta{};

  while (true) {
    auto [id, image] = receiver.receive(std::span<char>((char*)&meta, sizeof(meta)));

    stats_collector.processing_started();
    // I treat sending of the message as part of processing for now
    auto converted = converter.convert_data({(uint16_t*)image, MODULE_N_PIXELS});
    sender.send(id, std::span<char>((char*)&meta, sizeof(meta)), (char*)converted.data());

    stats_collector.processing_finished();
  }

  return 0;
}