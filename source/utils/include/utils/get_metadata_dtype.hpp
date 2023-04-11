/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_GET_METADATA_DTYPE_HPP
#define STD_DETECTOR_BUFFER_GET_METADATA_DTYPE_HPP

#include "std_daq/image_metadata.pb.h"

#include "detector_config.hpp"

namespace utils {

std_daq_protocol::ImageMetadataDtype get_metadata_dtype(const DetectorConfig& config)
{
  if (config.detector_type == "jungfrau") return std_daq_protocol::ImageMetadataDtype::float32;
  if (config.bit_depth == 8) return std_daq_protocol::ImageMetadataDtype::uint8;
  if (config.bit_depth == 16) return std_daq_protocol::ImageMetadataDtype::uint16;
  if (config.bit_depth == 32) return std_daq_protocol::ImageMetadataDtype::uint32;
  throw std::runtime_error("Unsupported bit_depth of the image");
}

std::size_t get_bytes_from_metadata_dtype(std_daq_protocol::ImageMetadataDtype type)
{
  if (type == std_daq_protocol::ImageMetadataDtype::float32) return 4;
  if (type == std_daq_protocol::ImageMetadataDtype::uint8) return 1;
  if (type == std_daq_protocol::ImageMetadataDtype::uint16) return 2;
  if (type == std_daq_protocol::ImageMetadataDtype::uint32) return 4;
  throw std::runtime_error("Unsupported bit_depth of the image");
}

std::string get_array10_type(std_daq_protocol::ImageMetadataDtype type)
{
  if (type == std_daq_protocol::ImageMetadataDtype::float32) return "float32";
  if (type == std_daq_protocol::ImageMetadataDtype::uint8) return "uint8";
  if (type == std_daq_protocol::ImageMetadataDtype::uint16) return "uint16";
  if (type == std_daq_protocol::ImageMetadataDtype::uint32) return "uint32";
  throw std::runtime_error("Unsupported type of the image");
}

} // namespace utils

#endif // STD_DETECTOR_BUFFER_GET_METADATA_DTYPE_HPP
