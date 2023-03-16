/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP
#define STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP

#include <iostream>
#include <unordered_map>

namespace utils {

using module_id = std::size_t;

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
  const std::unordered_map<module_id, std::pair<Point, Point>> modules;

  friend std::ostream& operator<<(std::ostream& os, DetectorConfig const& det_config)
  {
    return os << det_config.detector_name << ' ' << det_config.detector_type << ' '
              << det_config.n_modules << ' ' << det_config.bit_depth << ' '
              << det_config.image_pixel_height << ' ' << det_config.image_pixel_width << ' '
              << det_config.start_udp_port << ' ';
  }
};

DetectorConfig read_config_from_json_file(const std::string& filename);
DetectorConfig read_config_from_json_string(const std::string& data);

Point get_module_start_position(const DetectorConfig& config, module_id id);
Point get_module_end_position(const DetectorConfig& config, module_id id);

} // namespace utils

#endif // STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP
