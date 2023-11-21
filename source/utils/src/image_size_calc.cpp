/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "image_size_calc.hpp"

#include "detectors/gigafrost.hpp"

namespace utils {

std::size_t converted_image_n_bytes(const utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost")
    return gf::converted_image_n_bytes(config.image_pixel_height, config.image_pixel_width);
  if (config.detector_type == "eiger" || config.detector_type == "pco" ||
      config.detector_type == "jungfrau-raw")
    return config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  if (config.detector_type == "jungfrau-converted")
    return config.image_pixel_width * config.image_pixel_height * sizeof(float);
  throw std::runtime_error("Unsupported detector_type!\n");
}

std::size_t max_converted_image_byte_size(const utils::DetectorConfig& config)
{
  if (config.detector_type == "gigafrost") return 2016 * 2016 * 2;
  if (config.detector_type == "eiger") return 3106 * 3264 * 32 / 8;
  throw std::runtime_error("Unsupported detector_type!\n");
}

std::size_t max_single_sender_size(const utils::DetectorConfig& config)
{
  // TODO 8 should not be fixed for future - currently it assumes we always divide image into 8
  return max_converted_image_byte_size(config) / 8;
}

std::size_t number_of_senders(const utils::DetectorConfig& config)
{
  return (converted_image_n_bytes(config) + max_single_sender_size(config) - 1) /
         max_single_sender_size(config);
}

} // namespace utils
