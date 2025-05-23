/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include "std_buffer/image_metadata.pb.h"

namespace utils::stream {

inline std::string map_dtype_to_stream_type(std_daq_protocol::ImageMetadataDtype type)
{
  if (type == std_daq_protocol::ImageMetadataDtype::float32) return "float32";
  if (type == std_daq_protocol::ImageMetadataDtype::uint8) return "uint8";
  if (type == std_daq_protocol::ImageMetadataDtype::uint16) return "uint16";
  if (type == std_daq_protocol::ImageMetadataDtype::uint32) return "uint32";
  if (type == std_daq_protocol::ImageMetadataDtype::uint64) return "uint64";
  throw std::runtime_error("Unsupported bit_depth of the image");
}

} // namespace utils::stream
