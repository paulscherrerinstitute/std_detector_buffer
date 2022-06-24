#include "converter.hpp"

#include <fstream>

#include <fmt/core.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>

#include "buffer_utils.hpp"

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

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  auto [gains, pedestals] = read_gains_and_pedestals(argv[2]);

  sdc::Converter converter(gains, pedestals);

  return gains.size() + pedestals.size();
}
