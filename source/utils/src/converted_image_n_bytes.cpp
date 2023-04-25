/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converted_image_n_bytes.hpp"

#include "detectors/gigafrost.hpp"

namespace utils {

std::size_t converted_image_n_bytes(const utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost")
    return gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  if (config.detector_type == "eiger" || config.detector_type == "pco")
    return config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  throw std::runtime_error("Unsupported detector_type!\n");
}

} // namespace utils
