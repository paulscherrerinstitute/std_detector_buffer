#include <zmq.h>
#include <fmt/core.h>

#include "converter.hpp"
#include "read_gains_and_pedestals.hpp"
#include "stats_collector.hpp"

#include "jungfrau.hpp"
#include "buffer_utils.hpp"
#include "buffer_config.hpp"
#include "core_buffer/sender.hpp"
#include "core_buffer/receiver.hpp"

cb::Receiver create_receiver(uint16_t module_id,
                             const buffer_utils::DetectorConfig& config,
                             void* ctx)
{
  return cb::Receiver{
      {config.detector_name + "-" + std::to_string(module_id), BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
       DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME, buffer_config::RAM_BUFFER_N_SLOTS,
       static_cast<uint16_t>(config.start_udp_port + module_id)},
      ctx};
}

cb::Sender create_sender(uint16_t module_id, const buffer_utils::DetectorConfig& config, void* ctx)
{
  return cb::Sender{{config.detector_name + "-" + std::to_string(module_id) + "-converted",
                     BYTES_PER_PACKET - DATA_BYTES_PER_PACKET, MODULE_N_PIXELS * sizeof(float),
                     buffer_config::RAM_BUFFER_N_SLOTS,
                     static_cast<uint16_t>(config.start_udp_port + module_id)},
                    ctx};
}

void check_number_of_arguments(int argc)
{
  if (argc != 4) {
    fmt::print("Usage: std_data_convert [detector_json_filename] [gains_and_pedestal_h5_filename] "
               "[module_id]\n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tgains_and_pedestal_h5_filename: gains and pedestals h5 path.\n"
               "\tmodule_id: id of the module for this process.\n");
    exit(-1);
  }
}

sdc::Converter create_converter(const std::string& filename, std::size_t image_size)
{
  const auto [gains, pedestals] = sdc::read_gains_and_pedestals(filename, image_size);
  return sdc::Converter{gains, pedestals};
}

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  const uint16_t module_id = std::stoi(argv[3]);
  sdc::StatsCollector stats_collector(config.detector_name, module_id);

  auto converter = create_converter(argv[2], config.image_pixel_height * config.image_pixel_width);

  auto ctx = zmq_ctx_new();
  auto receiver = create_receiver(module_id, config, ctx);
  auto sender = create_sender(module_id, config, ctx);

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
