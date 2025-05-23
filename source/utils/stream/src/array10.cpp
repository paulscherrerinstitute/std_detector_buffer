/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <fmt/core.h>
#include <md5.h>

#include "array10.hpp"
#include "map_dtype_to_stream_type.hpp"

namespace utils::stream {

std::string prepare_array10_header(const std_daq_protocol::ImageMetadata& meta)
{
  return fmt::format(R"({{"htype":"array-1.0", "shape":[{},{}], "type":"{}", "frame":{}}})",
                             meta.height(), meta.width(), map_dtype_to_stream_type(meta.dtype()),
                             meta.image_id());
}

} // namespace utils::stream

