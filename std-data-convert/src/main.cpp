#include "converter.hpp"

#include <fstream>

#include <fmt/core.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>

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
  if (argc != 2) {
    fmt::print("Usage: std_data_convert [gains_and_pedestals_json]\n\n"
               "\tgains_and_pedestals_json: Json file containing gains .\n");
    exit(-1);
  }

  std::ifstream ifs(argv[1]);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json_data;
  json_data.ParseStream(isw);

  sdc::parameters gains = decode_parameters(json_data["gains"]);
  sdc::parameters pedestals = decode_parameters(json_data["pedestals"]);

  sdc::Converter converter(gains, pedestals);

  return gains.size() + pedestals.size();
}
