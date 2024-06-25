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
#include <fmt/ostream.h>

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
  const int max_number_of_forwarders_spawned;
  const bool use_all_forwarders;
  const int gpfs_block_size;
  const bool sender_sends_full_images;
  const int module_sync_queue_size;
  const int number_of_writers;
  const std::unordered_map<module_id, std::pair<Point, Point>> modules;

  friend std::ostream& operator<<(std::ostream& os, const DetectorConfig& det_config)
  {
    return os << fmt::format(
               "detector={},\ndetector_type={},\nn_modules={},\nbit_depth={},\n"
               "image_pixel_height={},\nimage_pixel_width={},\nstart_udp_port={},\n"
               "writer_user_id={},\nlog_level={},\nstats_collection_period={},\nmax_number_of_forwarders_"
               "spawned={},\nuse_all_forwarders={},\ngpfs_block_size={},\nsender_sends_full_images={},\n"
               "module_sync_queue_size={},\nnumber_of_writers={}\n",
               det_config.detector_name, det_config.detector_type, det_config.n_modules,
               det_config.bit_depth, det_config.image_pixel_height, det_config.image_pixel_width,
               det_config.start_udp_port, det_config.writer_user_id, det_config.log_level,
               det_config.stats_collection_period.count(),
               det_config.max_number_of_forwarders_spawned, det_config.use_all_forwarders,
               det_config.gpfs_block_size, det_config.sender_sends_full_images,
               det_config.module_sync_queue_size, det_config.number_of_writers);
  }
};

DetectorConfig read_config_from_json_file(const std::string& filename);
DetectorConfig read_config_from_json_string(const std::string& data);

Point get_module_start_position(const DetectorConfig& config, module_id id);
Point get_module_end_position(const DetectorConfig& config, module_id id);
modules_mask get_modules_mask(const DetectorConfig& config);
void test_if_module_is_inside_image(const utils::DetectorConfig& config, int module_id);

} // namespace utils

template <> struct fmt::formatter<utils::DetectorConfig> : ostream_formatter
{};

#endif // STD_DETECTOR_BUFFER_DETECTOR_CONFIG_HPP
