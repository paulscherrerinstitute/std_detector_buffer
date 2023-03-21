/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "utils/detector_config.hpp"

#include <gtest/gtest.h>

namespace utils {

TEST(DetectorConfig, ShouldReadModuleSettingsCorrectly)
{
  const std::string data = R""""({
"detector_name": "GF2",
"detector_type": "gigafrost",
"n_modules": 8,
"bit_depth": 16,
"image_pixel_height": 2016,
"image_pixel_width": 2016,
"start_udp_port": 50020,
"module_positions": { "0": [0, 1, 100, 101], "2": [1,2,3,4], "15": [5,6,7,8]}
}
)"""";

  const auto config = read_config_from_json_string(data);

  EXPECT_EQ("GF2", config.detector_name);
  EXPECT_EQ(8, config.n_modules);
  EXPECT_EQ(16, config.bit_depth);
  EXPECT_EQ(2016, config.image_pixel_width);
  EXPECT_EQ(2016, config.image_pixel_height);
  EXPECT_EQ(0, get_module_start_position(config, 0).x);
  EXPECT_EQ(1, get_module_start_position(config, 0).y);
  EXPECT_EQ(100, get_module_end_position(config, 0).x);
  EXPECT_EQ(101, get_module_end_position(config, 0).y);
  EXPECT_EQ(modules_mask{133}, get_modules_mask(config));
}

} // namespace utils
