/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <vector>
#include <span>

#include "utils/detector_config.hpp"

namespace jf::sdc {

class Converter
{
public:
  explicit Converter(const utils::DetectorConfig& config, int module_id);
  void convert(std::span<char> input_data, std::span<char> output_buffer) const;

private:
  static std::size_t calculate_row_jump(std::size_t image_width, std::size_t bit_depth);
  static int calculate_start_index(const utils::DetectorConfig& config, int module_id);
  static int calculate_row_jump_direction(const utils::DetectorConfig& config, int module_id);

  const std::size_t input_row_size;
  const int row_jump;
  const int start_index;
};

} // namespace eg::sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
