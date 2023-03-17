/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstring>

#include "converter.hpp"
#include "detectors/eiger.hpp"

namespace eg::sdc {

Converter::Converter(const utils::DetectorConfig& config, int module_id)
    : height(config.image_pixel_height)
    , width(config.image_pixel_width)
    , input_row_size(MODULE_X_SIZE * config.bit_depth / 8)
    , start_index(calculate_start_index(module_id, config.image_pixel_width, config.bit_depth))
    , row_jump(calculate_row_jump(0, config.image_pixel_width, config.bit_depth))
{}

void Converter::convert(std::span<char> input, std::span<char> output_buffer) const
{
  for (auto row = 0u; row < MODULE_Y_SIZE; row++) {
    const auto input_start = row * input_row_size;
    const auto output_start = start_index + row * row_jump;
    std::memcpy(output_buffer.data() + output_start, input.data() + input_start, input_row_size);
  }
}

int Converter::calculate_start_index(int module_id, std::size_t image_width, std::size_t bit_depth)
{
  return module_id % 2 == 0 ? 0 : (image_width - MODULE_X_SIZE) * bit_depth / 8;
}

int Converter::calculate_row_jump(int, std::size_t image_width, std::size_t bit_depth)
{
  return image_width * bit_depth / 8;
}

} // namespace eg::sdc
