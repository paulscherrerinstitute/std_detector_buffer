/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstring>

#include "converter.hpp"
#include "eiger.hpp"

namespace eg::sdc {

Converter::Converter(std::size_t image_height,
                     std::size_t image_width,
                     std::size_t depth,
                     int module_id)
    : height(image_height)
    , width(image_width)
    , input_row_size(MODULE_X_SIZE * depth / 8)
    , start_index(calculate_start_index(module_id, image_width, depth))
    , row_jump(calculate_row_jump(0, image_width, depth))
{}

void Converter::convert(std::span<char> input, std::span<char> output_buffer) const
{
  for (auto row = 0u; row < MODULE_Y_SIZE; row++) {
    const auto input_start = row * input_row_size;
    const auto output_start = start_index + row * row_jump;
    std::memcpy(output_buffer.data() + output_start, input.data() + input_start, input_row_size);
  }
}

int Converter::calculate_start_index(int module_id,
                                     std::size_t image_width,
                                     std::size_t bit_depth)
{
  return module_id % 2 == 0 ? 0 : (image_width - MODULE_X_SIZE) * bit_depth / 8;
}

int Converter::calculate_row_jump(int, std::size_t image_width, std::size_t bit_depth)
{
  return image_width * bit_depth / 8;
}

} // namespace eg::sdc