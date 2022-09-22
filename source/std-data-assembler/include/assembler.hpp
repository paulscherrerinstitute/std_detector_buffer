/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ASSEMBLER_HPP
#define STD_DETECTOR_BUFFER_ASSEMBLER_HPP

#include <cstdint>
#include <vector>
#include <span>

namespace sda {

class Assembler
{
public:
  explicit Assembler(std::size_t pixels_number);
  std::span<uint16_t> convert(std::span<char> data);
private:
  std::vector<uint16_t> converted;
};

} // namespace sda

#endif // STD_DETECTOR_BUFFER_ASSEMBLER_HPP
