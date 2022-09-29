/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <vector>
#include <span>

#include "parameters.hpp"

namespace sdc {

class Converter
{
public:
  explicit Converter(const parameters& g, const parameters& p);
  std::span<float> convert_data(std::span<const uint16_t> data);

private:
  void test_data_size_consistency(std::span<const uint16_t> data) const;
  std::span<float> convert(std::span<const uint16_t> data);

  parameters_pairs gains_and_pedestals;
  std::vector<float> converted;
};

} // namespace sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
