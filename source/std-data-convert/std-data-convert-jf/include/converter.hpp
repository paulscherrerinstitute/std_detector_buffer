/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

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

  explicit Converter(const utils::DetectorConfig& config, int module_id);
  void convert(std::span<const uint16_t> input, std::span<uint16_t>);

private:
  void convert(std::span<const uint16_t> input_data, std::span<float> output_data);
  void convert_data(std::span<const uint16_t> input_data, std::span<float> output_data);
  void copy_raw_data(std::span<const uint16_t> input, std::span<uint16_t> output_buffer) const;

  void test_data_size_consistency(std::span<const uint16_t> data) const;
  void test_if_module_size_fits_jungfrau(const utils::DetectorConfig& config, int module_id);
  static void test_gains_and_pedestals_consistency(const parameters& g, const parameters& p);
  static std::size_t calculate_start_index(const utils::DetectorConfig& config, int module_id);

  parameters_pairs gains_and_pedestals;
  std::size_t row_jump;
  std::size_t start_index;
  bool with_gains;
};

} // namespace jf::sdc
