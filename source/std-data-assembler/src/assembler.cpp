/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "assembler.hpp"

namespace sda {

namespace {
#pragma pack(1)
struct conversion_handle
{
  uint16_t p1 : 12;
  uint16_t p2 : 12;
  uint16_t p3 : 12;
  uint16_t p4 : 12;
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

  for (auto i = 0u, j = 0u; i < conversion.size(); i++, j += 4) {
    converted[j] = conversion[i].p1;
    converted[j + 1] = conversion[i].p2;
    converted[j + 2] = conversion[i].p3;
    converted[j + 3] = conversion[i].p4;
  }

  if(const auto )

  return converted;
}

} // namespace sda
