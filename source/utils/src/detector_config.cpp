/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "detector_config.hpp"

#include <fstream>
#include <unordered_map>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <fmt/core.h>

namespace utils {

namespace {
DetectorConfig read_config(const rapidjson::Document& doc)
{
  static const std::string required_parameters[] = {
      "detector_name",  "detector_type",      "n_modules",
      "bit_depth",      "image_pixel_height", "image_pixel_width",
      "start_udp_port", "module_positions",   "writer_user_id"};

  for (auto& s : required_parameters)
    if (!doc.HasMember(s.c_str()))
      throw std::runtime_error(fmt::format("Detector json file is missing parameter: \"{}\"", s));

  std::unordered_map<module_id, std::pair<Point, Point>> modules;
  for (auto& m : doc["module_positions"].GetObject())
    modules[std::stoi(m.name.GetString())] = {{m.value[0].GetInt(), m.value[1].GetInt()},
                                              {m.value[2].GetInt(), m.value[3].GetInt()}};

  return {doc["detector_name"].GetString(),
          doc["detector_type"].GetString(),
          doc["n_modules"].GetInt(),
          doc["bit_depth"].GetInt(),
          doc["image_pixel_height"].GetInt(),
          doc["image_pixel_width"].GetInt(),
          static_cast<uint16_t>(doc["start_udp_port"].GetUint()),
          doc["writer_user_id"].GetInt(),
          std::move(modules)};
}
} // namespace

DetectorConfig read_config_from_json_file(const std::string& filename)
{
  std::ifstream ifs(filename);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document doc;
  doc.ParseStream(isw);
  return read_config(doc);
}

DetectorConfig read_config_from_json_string(const std::string& data)
{
  rapidjson::Document doc;
  doc.Parse(data.c_str());
  return read_config(doc);
}

Point get_module_start_position(const DetectorConfig& config, module_id id)
{
  return config.modules.at(id).first;
}

Point get_module_end_position(const DetectorConfig& config, module_id id)
{
  return config.modules.at(id).second;
}

modules_mask get_modules_mask(const DetectorConfig& config)
{
  modules_mask m;
  for (auto [key, _] : config.modules)
    m.set(key % config.n_modules);
  return m;
}

} // namespace utils
