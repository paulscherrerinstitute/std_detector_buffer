/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstring>

#include <fmt/core.h>

#include "converter.hpp"
#include "detectors/eiger.hpp"

namespace eg::sdc {

Converter::Converter(const utils::DetectorConfig& config, int module_id)
    : input_row_size_per_chip(MODULE_X_SIZE * config.bit_depth / 16)
    , gap_size(2 * config.bit_depth / 8)
    , row_jump(static_cast<int>(calculate_row_jump(config.image_pixel_width, config.bit_depth)) *
               calculate_row_jump_direction(config, module_id))
    , start_index(calculate_start_index(config, module_id))
{
  if (auto diff = std::abs(utils::get_module_start_position(config, module_id).x -
                           utils::get_module_end_position(config, module_id).x);
      diff != MODULE_X_SIZE + 1)
    throw std::runtime_error(fmt::format("Eiger module width cannot be different than {} (set {})!",
                                         MODULE_X_SIZE, diff));
  if (auto diff = std::abs(utils::get_module_start_position(config, module_id).y -
                           utils::get_module_end_position(config, module_id).y);
      diff != MODULE_Y_SIZE - 1)
    throw std::runtime_error(fmt::format(
        "Eiger module height cannot be different than {} (set {})!", MODULE_Y_SIZE, diff));
}

void Converter::convert(std::span<char> input, std::span<char> output_buffer) const
{
  const auto input_second_chip_pos = input_row_size_per_chip;
  const auto output_second_chip_pos = input_second_chip_pos + gap_size;

  for (auto row = 0u; row < MODULE_Y_SIZE; row++) {
    const auto input_start = row * input_row_size_per_chip * 2;
    const auto output_start = start_index + row * row_jump;
    std::memcpy(output_buffer.data() + output_start, input.data() + input_start,
                input_row_size_per_chip);
    std::memcpy(output_buffer.data() + output_start + output_second_chip_pos,
                input.data() + input_start + input_second_chip_pos, input_row_size_per_chip);
  }
}

int Converter::calculate_start_index(const utils::DetectorConfig& config, int module_id)
{
  const auto start_position = utils::get_module_start_position(config, module_id);
  const auto row_jump = calculate_row_jump(config.image_pixel_width, config.bit_depth);
  return static_cast<int>(start_position.y * row_jump + start_position.x * config.bit_depth / 8);
}

std::size_t Converter::calculate_row_jump(std::size_t image_width, std::size_t bit_depth)
{
  return image_width * bit_depth / 8;
}

int Converter::calculate_row_jump_direction(const utils::DetectorConfig& config, int module_id)
{
  const auto start_position = utils::get_module_start_position(config, module_id);
  const auto end_position = utils::get_module_end_position(config, module_id);
  return end_position.y < start_position.y ? -1 : 1;
}

} // namespace eg::sdc
