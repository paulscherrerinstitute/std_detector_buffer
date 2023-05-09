/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"


class Synchronizer
{
  using image_id = uint64_t;
  using modules_mask = std::bitset<128>;

  const int n_modules;
  const size_t n_images_buffer;
  const modules_mask new_image_mask;
  std::map<image_id, std::pair<modules_mask, CommonFrame>> cache;

public:
  Synchronizer(int n_modules, int n_images_buffer, modules_mask mask = 0);
  // returns corrupted images
  std::size_t process_image_metadata(CommonFrame meta);
  CommonFrame pop_next_full_image();

private:
  static modules_mask set_all(int n_modules);
  std::size_t push_new_image_to_queue(CommonFrame meta);
  size_t update_module_mask_for_image(image_id id, size_t module_id);
  modules_mask& get_modules_mask_for_image(image_id id);
  uint32_t discard_stale_images(image_id id);
  void drop_oldest_incomplete_image();

  [[nodiscard]] bool is_new_image(image_id id) const;
  [[nodiscard]] bool is_queue_too_long() const;
};

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
