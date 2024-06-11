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
#include <mutex>
#include <variant>
#include <ranges>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"
#include "utils/image_id.hpp"

template <typename FrameType> class Synchronizer
{
  using modules_mask = std::bitset<128>;

  const int n_modules;
  const size_t n_images_buffer;
  const modules_mask new_image_mask;
  std::map<utils::image_id, std::pair<modules_mask, FrameType>> cache;
  mutable std::mutex mutex_cache;

public:
  Synchronizer(int n_modules, int n_images_buffer, modules_mask mask = 0)
      : n_modules(n_modules)
      , n_images_buffer(n_images_buffer)
      , new_image_mask(mask.none() ? set_all(n_modules) : mask)
      , cache()
  {}

  // returns corrupted images
  std::size_t process_image_metadata(FrameType meta)
  {
    std::lock_guard<std::mutex> lock(mutex_cache);
    if (is_new_image(meta.image_id))
      return push_new_image_to_queue(meta);
    else
      return update_module_mask_for_image(meta.image_id, meta.module_id);
  }

  std::optional<FrameType> pop_next_full_image()
  {
    std::lock_guard<std::mutex> lock(mutex_cache);
    if (cache.empty()) return std::nullopt;
    if (auto data = cache.begin()->second; data.first == 0) {
      cache.erase(cache.begin());
      return data.second;
    }
    return std::nullopt;
  }

private:
  static modules_mask set_all(int n_modules)
  {
    modules_mask m;
    for (int i : std::views::iota(0, n_modules))
      m.set(i);
    return m;
  }

  std::size_t push_new_image_to_queue(FrameType meta)
  {
    if (is_queue_too_long()) cache.erase(cache.begin());
    // Initialize the module mask to 1 for n_modules the least significant bits.
    auto mask = new_image_mask;
    mask.reset(meta.module_id % n_modules);
    cache.emplace(meta.image_id, std::make_pair(mask, meta));
    return is_queue_too_long();
  }

  size_t update_module_mask_for_image(utils::image_id id, size_t module_id)
  {
    auto& mask = get_modules_mask_for_image(id);

    // Has this module already arrived for this image_id?
    if (!mask.test(module_id % n_modules)) {
      cache.erase(id);
      return 1;
    }
    else {
      // Clear bit in 'part_id' place.
      mask.reset(module_id % n_modules);
      return 0;
    }
  }

  modules_mask& get_modules_mask_for_image(utils::image_id id)
  {
    return cache.find(id)->second.first;
  }

  [[nodiscard]] bool is_new_image(utils::image_id id) const
  {
    return cache.find(id) == cache.end();
  }

  [[nodiscard]] bool is_queue_too_long() const { return cache.size() > n_images_buffer; }
};

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
