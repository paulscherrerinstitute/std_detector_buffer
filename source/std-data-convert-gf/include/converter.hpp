/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <vector>
#include <span>

namespace gf::sdc {

class Converter
{
public:
  explicit Converter(std::size_t image_height, std::size_t image_width);
  void convert(std::span<char> input_data, std::span<char> output_buffer, int module);

private:
  const std::size_t height;
  const std::size_t width;
  const std::size_t pixels;
};

} // namespace gf::sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
