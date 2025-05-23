/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <tuple>

#include "std_buffer/image_metadata.pb.h"

namespace utils::stream {

std::string prepare_array10_header(const std_daq_protocol::ImageMetadata& meta);

} // namespace utils::stream
