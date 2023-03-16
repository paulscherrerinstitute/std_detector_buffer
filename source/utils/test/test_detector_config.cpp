/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "utils/detector_config.hpp"

#include <chrono>

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
"start_udp_port": 50020
}
)"""";

  const auto config = read_config_from_json_string(data);

  EXPECT_EQ("GF2", config.detector_name);
  EXPECT_EQ(8, config.n_modules);
  EXPECT_EQ(16, config.bit_depth);
  EXPECT_EQ(2016, config.image_pixel_width);
  EXPECT_EQ(2016, config.image_pixel_height);
}

} // namespace utils
