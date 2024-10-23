/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "image_size_calc.hpp"

namespace utils {

std::size_t converted_image_n_bytes(const DetectorConfig& config)
{
  if (config.detector_type == "gigafrost")
    return config.image_pixel_height * config.image_pixel_width * 2;
  if (config.detector_type == "eiger" || config.detector_type == "pco" ||
      config.detector_type == "jungfrau-raw")
    return config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  if (config.detector_type == "jungfrau-converted")
    return config.image_pixel_width * config.image_pixel_height * sizeof(float);
  throw std::runtime_error("Unsupported detector_type!\n");
}

std::size_t max_converted_image_byte_size(const DetectorConfig& config)
{
  if (config.detector_type == "gigafrost") return 2016 * 2016 * sizeof(uint16_t);
  if (config.detector_type == "eiger") return 3106 * 3264 * sizeof(uint32_t);
  if (config.detector_type == "jungfrau-raw") return 1024 * 512 * 32 * sizeof(uint16_t);
  if (config.detector_type == "jungfrau-converted") return 1024 * 512 * 32 * sizeof(float);
  throw std::runtime_error("Unsupported detector_type!\n");
}

std::size_t slots_number(const DetectorConfig& config)
{
  constexpr std::size_t MIN_RAM_BUFFER_N_SLOTS = 10 * 100u;
  const std::size_t slots =
      config.ram_buffer_gb * 1024 * 1024 * 1024 / converted_image_n_bytes(config);
  return std::max(slots, MIN_RAM_BUFFER_N_SLOTS);
}

std::size_t calculate_image_offset(const DetectorConfig& config, const std::size_t image_part)
{
  if (config.sender_sends_full_images)
    return 0;
  else if (config.use_all_forwarders)
    return image_part * converted_image_n_bytes(config) / config.max_number_of_forwarders_spawned;
  else
    return image_part * max_converted_image_byte_size(config) /
           config.max_number_of_forwarders_spawned;
}

std::size_t max_single_sender_size(const DetectorConfig& config)
{
  if (config.sender_sends_full_images)
    return converted_image_n_bytes(config);
  else if (config.use_all_forwarders)
    return converted_image_n_bytes(config) / config.max_number_of_forwarders_spawned;
  else
    return max_converted_image_byte_size(config) / config.max_number_of_forwarders_spawned;
}

std::size_t calculate_image_bytes_sent(const DetectorConfig& config, std::size_t image_part)
{
  const auto converted_bytes = converted_image_n_bytes(config);
  const auto start_index = calculate_image_offset(config, image_part);
  return std::min(converted_bytes - start_index, max_single_sender_size(config));
}

std::size_t number_of_senders(const DetectorConfig& config)
{
  if (config.sender_sends_full_images)
    return 1;
  else if (config.use_all_forwarders)
    return config.max_number_of_forwarders_spawned;
  else
    return (converted_image_n_bytes(config) + max_single_sender_size(config) - 1) /
           max_single_sender_size(config);
}

} // namespace utils
