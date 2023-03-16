/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "detector_config.hpp"

#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <fmt/core.h>

namespace utils {

DetectorConfig read_config_from_json_file(const std::string& filename)
{
  std::ifstream ifs(filename);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document config_parameters;
  config_parameters.ParseStream(isw);

  static const std::string required_parameters[] = {
      "detector_name",      "detector_type",     "n_modules",     "bit_depth",
      "image_pixel_height", "image_pixel_width", "start_udp_port"};

  for (auto& s : required_parameters)
    if (!config_parameters.HasMember(s.c_str()))
      throw std::runtime_error(fmt::format("Detector json file is missing parameter: \"{}\"", s));

  return {config_parameters["detector_name"].GetString(),
          config_parameters["detector_type"].GetString(),
          config_parameters["n_modules"].GetInt(),
          config_parameters["bit_depth"].GetInt(),
          config_parameters["image_pixel_height"].GetInt(),
          config_parameters["image_pixel_width"].GetInt(),
          static_cast<uint16_t>(config_parameters["start_udp_port"].GetUint())};
}

} // namespace utils
