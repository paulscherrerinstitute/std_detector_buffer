/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_IMAGE_SIZE_CALC_HPP
#define STD_DETECTOR_BUFFER_IMAGE_SIZE_CALC_HPP

#include "detector_config.hpp"

namespace utils {
std::size_t converted_image_n_bytes(const utils::DetectorConfig& config);
} // namespace utils

#endif // STD_DETECTOR_BUFFER_IMAGE_SIZE_CALC_HPP
