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

int main(int argc, char* argv[])
{
  if (argc != 3) {
    fmt::print("Usage: std_data_convert [detector_json_filename] [module_id]\n\n"
               "\tdetector_json_filename: detector config file path.\n"
               "\tmodule_id: id of the module for this process.\n");
    exit(-1);
  }

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  std::ifstream ifs(argv[1]);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json_data;
  json_data.ParseStream(isw);

  sdc::parameters gains = decode_parameters(json_data["gains"]);
  sdc::parameters pedestals = decode_parameters(json_data["pedestals"]);

  sdc::Converter converter(gains, pedestals);

  return gains.size() + pedestals.size();
}
