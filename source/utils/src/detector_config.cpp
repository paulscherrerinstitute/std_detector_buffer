/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "detector_config.hpp"

#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace utils {

namespace {
DetectorConfig read_config(const json doc)
{
  static const std::string required_parameters[] = {
      "detector_name",  "detector_type",      "n_modules",
      "bit_depth",      "image_pixel_height", "image_pixel_width",
      "start_udp_port", "module_positions",   "writer_user_id"};

  std::ranges::for_each(required_parameters, [&doc](const auto& s) {
    if (!doc.contains(s))
      throw std::runtime_error(fmt::format("Detector json file is missing parameter: \"{}\"", s));
  });

  std::unordered_map<module_id, std::pair<Point, Point>> modules;
  for (const auto& [key, value] : doc["module_positions"].items()) {
    if (value.size() != 4)
      throw std::runtime_error(fmt::format("Module position for key {} should have 4 values", key));
    modules[std::stoi(key)] = {{value[0].get<int>(), value[1].get<int>()},
                               {value[2].get<int>(), value[3].get<int>()}};
  }

  return {doc["detector_name"].get<std::string>(),
          doc["detector_type"].get<std::string>(),
          doc["n_modules"].get<int>(),
          doc["bit_depth"].get<int>(),
          doc["image_pixel_height"].get<int>(),
          doc["image_pixel_width"].get<int>(),
          doc["start_udp_port"].get<uint16_t>(),
          doc["writer_user_id"].get<int>(),
          doc.value("log_level", "info"),
          std::chrono::seconds(doc.value("stats_collection_period", 10)),
          doc.value("max_number_of_forwarders_spawned", 8),
          doc.value("use_all_forwarders", false),
          doc.value("gpfs_block_size", 16777216),
          std::move(modules)};
}
} // namespace

DetectorConfig read_config_from_json_file(const std::string& filename)
{
  std::ifstream ifs(filename);
  return read_config(json::parse(ifs));
}

DetectorConfig read_config_from_json_string(const std::string& data)
{
  return read_config(json::parse(data));
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

void test_if_module_is_inside_image(const utils::DetectorConfig& config, int module_id)
{
  const auto module_end = get_module_end_position(config, module_id);
  const auto module_start = get_module_start_position(config, module_id);

  if (module_start.x > config.image_pixel_width || module_start.y > config.image_pixel_height)
    throw std::runtime_error(fmt::format(
        "Start of module is out-of-bound! start({}, {}), image size({}, {})!", module_start.x,
        module_start.y, config.image_pixel_width, config.image_pixel_height));
  if (module_end.x > config.image_pixel_width || module_end.y > config.image_pixel_height)
    throw std::runtime_error(fmt::format(
        "End of module is out-of-bound! start({}, {}), image size({}, {})!", module_end.x,
        module_end.y, config.image_pixel_width, config.image_pixel_height));
}

} // namespace utils
