#include "read_gains_and_pedestals.hpp"

#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>

namespace sdc {
namespace {

parameters decode_parameters(const rapidjson::Value& array_3d)
{
  parameters params;
  for (auto i = 0u; i < params.size(); i++) {
    auto& params_line = params[i];
    for (const auto& array_1d : array_3d[i].GetArray())
      for (const auto& value : array_1d.GetArray())
        params_line.push_back(value.GetFloat()); // currently these are saved as doubles
  }
  return params;
}
} // namespace

std::tuple<parameters, parameters> read_gains_and_pedestals(std::string_view filename)
{
  std::ifstream ifs(filename.data());
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document json_data;
  json_data.ParseStream(isw);

  return {decode_parameters(json_data["gains"]), decode_parameters(json_data["pedestals"])};
}
} // namespace sdc
