/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_RUN_SETTINGS_HPP
#define STD_DETECTOR_BUFFER_RUN_SETTINGS_HPP

#include <string>

namespace std_driver {

using writer_id = int32_t;

struct run_settings
{
  std::string path;
  std::size_t n_images;
  writer_id writer;
  std::size_t start_image_id;
};
} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_RUN_SETTINGS_HPP
