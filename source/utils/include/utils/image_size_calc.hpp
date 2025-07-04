/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "detector_config.hpp"

namespace utils {

std::size_t converted_image_n_bytes(const utils::DetectorConfig& config);
std::size_t max_converted_image_byte_size(const utils::DetectorConfig& config);
std::size_t slots_number(const utils::DetectorConfig& config);
std::size_t calculate_image_offset(const utils::DetectorConfig& config, std::size_t image_part);
std::size_t calculate_image_bytes_sent(const utils::DetectorConfig& config, std::size_t image_part);
std::size_t number_of_senders(const utils::DetectorConfig& config);

} // namespace utils
