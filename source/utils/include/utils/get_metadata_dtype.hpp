/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "std_buffer/image_metadata.pb.h"

#include "detector_config.hpp"

namespace utils {

inline std_daq_protocol::ImageMetadataDtype get_metadata_dtype(const DetectorConfig& config)
{
  if (config.detector_type == "jungfrau-converted")
    return std_daq_protocol::ImageMetadataDtype::float32;
  if (config.bit_depth == 8) return std_daq_protocol::ImageMetadataDtype::uint8;
  if (config.bit_depth == 16) return std_daq_protocol::ImageMetadataDtype::uint16;
  if (config.bit_depth == 32) return std_daq_protocol::ImageMetadataDtype::uint32;
  throw std::runtime_error("Unsupported bit_depth of the image");
}

inline std::size_t get_bytes_from_metadata_dtype(std_daq_protocol::ImageMetadataDtype type)
{
  if (type == std_daq_protocol::ImageMetadataDtype::float32) return 4;
  if (type == std_daq_protocol::ImageMetadataDtype::uint8) return 1;
  if (type == std_daq_protocol::ImageMetadataDtype::uint16) return 2;
  if (type == std_daq_protocol::ImageMetadataDtype::uint32) return 4;
  throw std::runtime_error("Unsupported bit_depth of the image");
}

} // namespace utils
