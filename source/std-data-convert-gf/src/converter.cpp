/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

namespace gf::sdc {

namespace {

#pragma pack(1)
struct conversion_handle
{
  int p11 : 8;
  int p21 : 4;
  int p12 : 4;
  int p22 : 8;
};
#pragma pack()
} // namespace

Converter::Converter(std::size_t image_height, std::size_t image_width, quadrant_id q)
    : height(image_height)
    , width(image_width)
    , start_index(calculate_start_index(q, image_height, image_width))
    , row_jump(calculate_row_jump(q, image_width))
{}

void Converter::convert(std::span<char> input, std::span<char> output_buffer, int module)
{
  const auto start_index_for_module = start_index + (module % 2 == 0 ? row_jump / 2 : 0);
  // in theory the pixels number should be divisible by 4 - so there should be no problem with
  // alignment of conversion_handle
  const std::span<conversion_handle> conversion(reinterpret_cast<conversion_handle*>(input.data()),
                                                input.size() / sizeof(conversion_handle));

  std::span<uint16_t> output(reinterpret_cast<uint16_t*>(output_buffer.data()),
                             output_buffer.size() / sizeof(uint16_t));

  for (auto i = 0u, row = 0u; row < height / 4; row++) {
    const auto start_row = start_index_for_module + (row_jump * row);
    const auto end_row = start_row + width / 2;

    for (auto j = start_row; j < end_row; i++, j += 2) {
      output[j] = (conversion[i].p11 << 4) | conversion[i].p12;
      output[j + 1] = (conversion[i].p21 << 8) | conversion[i].p22;
    }
  }
}

int Converter::calculate_start_index(quadrant_id quadrant,
                                     std::size_t image_height,
                                     std::size_t image_width)
{
  if (quadrant == quadrant_id::SW) return image_height / 2 * image_width;
  if (quadrant == quadrant_id::SE) return (image_height / 2 * image_width) + (image_width / 2);
  if (quadrant == quadrant_id::NW) return (image_height - 1) / 2 * image_width;
  return (image_height - 1) / 2 * image_width + (image_width / 2);
}

int Converter::calculate_row_jump(quadrant_id quadrant, std::size_t image_width)
{
  return image_width * 2 * (quadrant == quadrant_id::NW || quadrant == quadrant_id::NE ? -1 : 1);
}

} // namespace gf::sdc
