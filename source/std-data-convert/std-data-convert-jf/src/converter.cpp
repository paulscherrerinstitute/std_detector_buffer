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
    : Converter(config, module_id)
{
  with_gains = true;
  test_gains_and_pedestals_consistency(g, p);

  for (auto i = 0u; i < N_GAINS; i++) {
    gains_and_pedestals[i].reserve(g[i].size());
    std::ranges::transform(g[i], p[i], std::back_inserter(gains_and_pedestals[i]),
                           [](auto a, auto b) { return std::make_pair(a, b); });
  }
}

Converter::Converter(const utils::DetectorConfig& config, int module_id)
    : row_jump(config.image_pixel_width)
    , start_index(calculate_start_index(config, module_id))
    , with_gains(false)
{
  utils::test_if_module_is_inside_image(config, module_id);
  test_if_module_size_fits_jungfrau(config, module_id);
}

void Converter::convert(std::span<const uint16_t> input, std::span<uint16_t> output)
{
  if (with_gains)
    convert_data(input, {(float*)output.data(), output.size() / sizeof(float) * sizeof(uint16_t)});
  else
    copy_raw_data(input, output);
}

void Converter::copy_raw_data(std::span<const uint16_t> input,
                              std::span<uint16_t> output_buffer) const
{
  for (auto row = 0u; row < MODULE_Y_SIZE; row++) {
    const auto input_start = row * MODULE_X_SIZE;
    const auto output_start = (start_index + row * row_jump);
    std::memcpy(output_buffer.data() + output_start, input.data() + input_start,
                MODULE_X_SIZE * sizeof(uint16_t));
  }
}

void Converter::convert_data(std::span<const uint16_t> input_data, std::span<float> output_data)
{
  test_data_size_consistency(input_data);
  convert(input_data, output_data);
}

void Converter::convert(std::span<const uint16_t> input_data, std::span<float> output_data)
{
  for (auto i = 0u; i < MODULE_Y_SIZE; i++) {
    for (auto j = 0u; j < MODULE_X_SIZE; j++) {
      auto index = i * MODULE_X_SIZE + j;
      auto gain_group = (input_data[index] >> 14) % N_GAINS;
      output_data[start_index + (i * row_jump) + j] =
          ((input_data[index] & 0x3FFF) - gains_and_pedestals[gain_group][index].second) *
          gains_and_pedestals[gain_group][index].first;
    }
  }
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
