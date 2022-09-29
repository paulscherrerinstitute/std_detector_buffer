/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "converter.hpp"
#include "read_gains_and_pedestals.hpp"
#include "stats_collector.hpp"
#include "identifier.hpp"

#include "jungfrau.hpp"
#include "buffer_utils.hpp"
#include "buffer_config.hpp"
#include "core_buffer/sender.hpp"
#include "core_buffer/receiver.hpp"

cb::Receiver create_receiver(std::string name, void* ctx)
{
  return cb::Receiver{{std::move(name), BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
                       DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME,
                       buffer_config::RAM_BUFFER_N_SLOTS},
                      ctx};
}

cb::Sender create_sender(std::string name, void* ctx)
{
  return cb::Sender{{std::move(name), BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
                     MODULE_N_PIXELS * sizeof(float), buffer_config::RAM_BUFFER_N_SLOTS},
                    ctx};
}

void check_number_of_arguments(int argc)
{
  if (argc != 5) {
    fmt::print("Usage: std_data_convert_jf [detector_json_filename] [gains_and_pedestal_h5_filename] "
               "[module_id] [converter_index]\n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tgains_and_pedestal_h5_filename: gains and pedestals h5 path.\n"
               "\tmodule_id: id of the module for this process.\n"
               "\tconverter_index: index of converter used to determine the output.\n");
    exit(-1);
  }
}

sdc::jf::Converter create_converter(const std::string& filename, std::size_t image_size)
{
  const auto [gains, pedestals] = sdc::jf::read_gains_and_pedestals(filename, image_size);
  return sdc::jf::Converter{gains, pedestals};
}

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  const uint16_t module_id = std::stoi(argv[3]);
  const uint16_t converter_index = std::stoi(argv[4]);
  const sdc::jf::Identifier converter_id(config.detector_name, module_id, converter_index);
  sdc::jf::StatsCollector stats_collector(converter_id);

  auto converter = create_converter(argv[2], config.image_pixel_height * config.image_pixel_width);

  auto ctx = zmq_ctx_new();
  auto receiver = create_receiver(converter_id.source_name(), ctx);
  auto sender = create_sender(converter_id.converter_name(), ctx);

  while (true) {
    auto [id, meta, image] = receiver.receive();

    stats_collector.processing_started();
    // I treat sending of the message as part of processing for now
    auto converted = converter.convert_data({(uint16_t*)image, MODULE_N_PIXELS});
    sender.send(id, meta, (char*)converted.data());

    stats_collector.processing_finished();
  }

  return 0;
}
