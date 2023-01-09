/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

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
  void convert(std::span<char> input_data, std::span<char> output_buffer);

private:
  static int calculate_start_index(int module_id,
                                   quadrant_id quadrant,
                                   std::size_t image_height,
                                   std::size_t image_width);
  static int calculate_row_jump(quadrant_id quadrant, std::size_t image_width);

  const std::size_t height;
  const std::size_t width;
  const int start_index;
  const int row_jump;
};

} // namespace gf::sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
