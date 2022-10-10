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

Converter::Converter(std::size_t image_height, std::size_t image_width)
    : height(image_height)
    , width(image_width)
    , pixels(image_width * image_height)
{}

void Converter::convert(std::span<char> input, std::span<char> output_buffer, int module)
{
  const auto start_index = height / 2 * width + (module % 2 == 0) * width;
  const auto row_jump = width * 2;
  // in theory the pixels number should be divisible by 4 - so there should be no problem with
  // alignment of conversion_handle
  const std::span<conversion_handle> conversion(reinterpret_cast<conversion_handle*>(input.data()),
                                                input.size() / sizeof(conversion_handle));

  std::span<uint16_t> output(reinterpret_cast<uint16_t*>(output_buffer.data()),
                             output_buffer.size() / sizeof(uint16_t));

  for(auto i = 0u, row = 0u; row < height / 4; row++)
  {
    const auto start_row = start_index + (row_jump * i);
    const auto end_row = start_row + width / 2;

    for(auto j = start_row; j < end_row; i++, j += 2)
    {
      output[j] = (conversion[i].p11 << 4) | conversion[i].p12;
      output[j + 1] = (conversion[i].p21 << 8) | conversion[i].p22;
    }
  }
}

} // namespace gf::sdc
