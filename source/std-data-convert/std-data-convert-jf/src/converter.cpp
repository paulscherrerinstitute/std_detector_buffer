/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <algorithm>

#include <fmt/core.h>
#include "detectors/jungfrau.hpp"

namespace jf::sdc {

Converter::Converter(const parameters& g,
                     const parameters& p,
                     const utils::DetectorConfig& config,
                     int module_id)
    : converted(MODULE_X_SIZE * MODULE_Y_SIZE)
    , row_jump(config.image_pixel_width)
    , start_index(calculate_start_index(config, module_id))
{
  utils::test_if_module_is_inside_image(config, module_id);
  test_if_module_size_fits_jungfrau(config, module_id);
  test_gains_and_pedestals_consistency(g, p);

  for (auto i = 0u; i < N_GAINS; i++) {
    gains_and_pedestals[i].reserve(g[i].size());
    std::ranges::transform(g[i], p[i], std::back_inserter(gains_and_pedestals[i]),
                           [](auto a, auto b) { return std::make_pair(a, b); });
  }
}

void Converter::convert_data(std::span<const uint16_t> input_data, std::span<float> output_data)
{
  test_data_size_consistency(input_data);
  convert(input_data);
  copy_converted_data(output_data);
}

void Converter::convert(std::span<const uint16_t> data)
{
  for (auto i = 0u; i < data.size(); i++) {
    auto gain_group = data[i] >> 14;
    converted[i] = ((data[i] & 0x3FFF) - gains_and_pedestals[gain_group][i].second) *
                   gains_and_pedestals[gain_group][i].first;
  }
}

void Converter::copy_converted_data(std::span<float> output_data)
{
  for (auto i = 0u; i < MODULE_Y_SIZE; i++)
    memcpy(output_data.data() + start_index + (i * row_jump),
           converted.data() + (i * MODULE_X_SIZE), MODULE_X_SIZE * sizeof(float));
}

void Converter::test_data_size_consistency(std::span<const uint16_t> data) const
{
  if (data.size() > gains_and_pedestals[0].size())
    throw std::invalid_argument(
        fmt::format("data size is greater than gains/pedestal arrays size - (expected {} != {})",
                    gains_and_pedestals[0].size(), data.size()));
}

void Converter::test_gains_and_pedestals_consistency(const parameters& g, const parameters& p)
{
  if (g[0].size() != p[0].size())
    throw std::runtime_error(
        fmt::format("Gains and pedestals have different size {} != {}!", g[0].size(), p[0].size()));
}

void Converter::test_if_module_size_fits_jungfrau(const utils::DetectorConfig& config,
                                                  int module_id)
{
  const auto module_end = utils::get_module_end_position(config, module_id);
  const auto module_start = utils::get_module_start_position(config, module_id);

  if (auto diff = abs(module_start.x - module_end.x); diff != MODULE_X_SIZE - 1)
    throw std::runtime_error(fmt::format(
        "Jungfrau module width cannot be different than {} (set {})!", MODULE_X_SIZE - 1, diff));
  if (auto diff = abs(module_start.y - module_end.y); diff != MODULE_Y_SIZE - 1)
    throw std::runtime_error(fmt::format(
        "Jungfrau module height cannot be different than {} (set {})!", MODULE_Y_SIZE - 1, diff));
}

std::size_t Converter::calculate_start_index(const utils::DetectorConfig& config, int module_id)
{
  const auto start_position = utils::get_module_start_position(config, module_id);
  return (config.image_pixel_width * start_position.y) + start_position.x;
}

} // namespace jf::sdc
