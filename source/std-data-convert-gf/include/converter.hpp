/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <vector>
#include <span>

namespace sdc::gf {

class Converter
{
public:
  explicit Converter(std::size_t pixels_number);
  std::span<uint16_t> convert(std::span<char> data);
private:
  std::vector<uint16_t> converted;
};

} // namespace sdc::gf

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
