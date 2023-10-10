/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP
#define STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP

#include <iostream>
#include <unordered_map>
#include <bitset>
#include <chrono>

#include <fmt/core.h>

namespace utils {

using module_id = std::size_t;
using modules_mask = std::bitset<128>;

struct Point
{
  int x = 0;
  int y = 0;
};

struct DetectorConfig
{
  const std::string detector_name;
  const std::string detector_type;
  const int n_modules;
  const int bit_depth;
  const int image_pixel_height;
  const int image_pixel_width;
  const uint16_t start_udp_port;
  const int writer_user_id;
  const std::string log_level;
  const std::chrono::seconds stats_collection_period;
  const std::unordered_map<module_id, std::pair<Point, Point>> modules;

  friend std::ostream& operator<<(std::ostream& os, DetectorConfig const& det_config)
  {
    return os << fmt::format("detector={},detector_type={},n_modules={},bit_depth={},"
                             "image_pixel_height={},image_pixel_width={},start_udp_port={},"
                             "writer_user_id={},log_level={},stats_collection_period={}",
                             det_config.detector_name, det_config.detector_type,
                             det_config.n_modules, det_config.bit_depth,
                             det_config.image_pixel_height, det_config.image_pixel_width,
                             det_config.start_udp_port, det_config.writer_user_id,
                             det_config.log_level, det_config.stats_collection_period.count());
  }
};

DetectorConfig read_config_from_json_file(const std::string& filename);
DetectorConfig read_config_from_json_string(const std::string& data);

Point get_module_start_position(const DetectorConfig& config, module_id id);
Point get_module_end_position(const DetectorConfig& config, module_id id);
modules_mask get_modules_mask(const DetectorConfig& config);

} // namespace utils

#endif // STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP
