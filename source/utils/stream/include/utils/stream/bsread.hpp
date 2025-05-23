/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <tuple>

#include "std_buffer/image_metadata.pb.h"

namespace utils::stream {

std::tuple<std::string, std::string> prepare_bsread_headers(const std_daq_protocol::ImageMetadata& meta);

} // namespace utils::stream
