/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <vector>
#include <span>

#include "detectors/gigafrost.hpp"

namespace gf::sdc {

class Converter
{
public:
  explicit Converter(std::size_t image_height,
                     std::size_t image_width,
                     quadrant_id q,
                     int module_id);
  void convert(std::span<char> input_data, std::span<char> output_buffer) const;

private:
  static std::size_t calculate_start_index(int module_id,
                                           quadrant_id quadrant,
                                           std::size_t image_height,
                                           std::size_t image_width);
  static int calculate_row_jump(quadrant_id quadrant, std::size_t image_width);

  const std::size_t height;
  const std::size_t width;
  const std::size_t start_index;
  const int row_jump;
};

} // namespace gf::sdc
