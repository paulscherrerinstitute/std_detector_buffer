/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <vector>
#include <span>

#include "utils/detector_config.hpp"

#include "parameters.hpp"

namespace jf::sdc {

class Converter
{
public:
  explicit Converter(const parameters& g,
                     const parameters& p,
                     const utils::DetectorConfig& config,
                     int module_id);

  void convert_data(std::span<const uint16_t> input_data, std::span<float> output_data);

private:
  void convert(std::span<const uint16_t> data);
  void copy_converted_data(std::span<float> output_data);

  void test_data_size_consistency(std::span<const uint16_t> data) const;
  void test_if_module_size_fits_jungfrau(const utils::DetectorConfig& config, int module_id);
  static void test_gains_and_pedestals_consistency(const parameters& g, const parameters& p);
  static std::size_t calculate_start_index(const utils::DetectorConfig& config, int module_id);

  parameters_pairs gains_and_pedestals;
  std::vector<float> converted;
  std::size_t row_jump;
  std::size_t start_index;
};

} // namespace jf::sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
