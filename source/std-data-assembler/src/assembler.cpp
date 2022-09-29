/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "assembler.hpp"

namespace sda {

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

Assembler::Assembler(std::size_t pixels_number)
    : converted(pixels_number)
{}

std::span<uint16_t> Assembler::convert(std::span<char> input)
{
  // in theory the pixels number should be divisible by 4 - so there should be no problem with
  // alignment of conversion_handle
  const std::span<conversion_handle> conversion(reinterpret_cast<conversion_handle*>(input.data()),
                                                input.size() / sizeof(conversion_handle));

  for (auto i = 0u, j = 0u; i < conversion.size(); i++, j += 2) {
    converted[j] = (conversion[i].p11 << 4) | conversion[i].p12;
    converted[j + 1] = (conversion[i].p21 << 8) | conversion[i].p22;
  }

  return converted;
}

} // namespace sda
