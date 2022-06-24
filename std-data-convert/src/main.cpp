#include "converter.hpp"

#include <fstream>

#include <zmq.h>
#include <fmt/core.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>

#include "buffer_utils.hpp"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "core_buffer/sender.hpp"
#include "core_buffer/receiver.hpp"

sdc::parameters decode_parameters(const rapidjson::Value& array_3d)
{
  sdc::parameters params;
  for (auto i = 0u; i < params.size(); i++) {
    auto& params_line = params[i];
    for (const auto& array_1d : array_3d[i].GetArray())
      for (const auto& value : array_1d.GetArray())
        params_line.push_back(value.GetFloat()); // currently these are saved as doubles
  }
  return params;
}

std::tuple<sdc::parameters, sdc::parameters> read_gains_and_pedestals(std::string_view filename)
{
  std::ifstream ifs(filename.data());
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json_data;
  json_data.ParseStream(isw);

  return {decode_parameters(json_data["gains"]), decode_parameters(json_data["pedestals"])};
}

cb::Receiver create_receiver(uint16_t module_id,
                             const buffer_utils::DetectorConfig& config,
                             void* ctx)
{
  return cb::Receiver{
      {config.detector_name + std::to_string(module_id), BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
       DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME, buffer_config::RAM_BUFFER_N_SLOTS},
      ctx};
}

cb::Sender create_sender(uint16_t module_id, const buffer_utils::DetectorConfig& config, void* ctx)
{
  return cb::Sender{{config.detector_name + std::to_string(module_id) + "-converted",
                     BYTES_PER_PACKET - DATA_BYTES_PER_PACKET, MODULE_N_PIXELS * sizeof(float),
                     buffer_config::RAM_BUFFER_N_SLOTS},
                    ctx};
}

int main(int argc, char* argv[])
{
  if (argc != 4) {
    fmt::print("Usage: std_data_convert [detector_json_filename] [gains_and_pedestals_json] "
               "[module_id]\n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tgains_and_pedestals_json: gains and pedestals json path.\n"
               "\tmodule_id: id of the module for this process.\n");
    exit(-1);
  }

  const uint16_t module_id = std::stoi(argv[2]);
  const auto config = buffer_utils::read_json_config(std::string(argv[1]));

  auto [gains, pedestals] = read_gains_and_pedestals(argv[2]);
  sdc::Converter converter(gains, pedestals);

  auto ctx = zmq_ctx_new();
  auto receiver = create_receiver(module_id, config, ctx);
  auto sender = create_sender(module_id, config, ctx);

  while (true) {
    auto [id, meta, image] = receiver.receive();
    auto converted = converter.convert_data({(uint16_t*)image, MODULE_N_PIXELS});
    sender.send(id, meta, (char*)converted.data());
  }

  return 0;
}
