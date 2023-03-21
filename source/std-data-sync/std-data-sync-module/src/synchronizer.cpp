/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <stdexcept>
#include <ranges>

#include <zmq.h>

using namespace std;

Synchronizer::Synchronizer(int n_modules, int n_images_buffer, modules_mask mask)
    : n_modules(n_modules)
    , n_images_buffer(n_images_buffer)
    , new_image_mask(mask.none() ? set_all(n_modules) : mask)
    , cache()
{}

std::size_t Synchronizer::process_image_metadata(CommonFrame meta)
{
  if (is_new_image(meta.image_id))
    return push_new_image_to_queue(meta);
  else
    return update_module_mask_for_image(meta.image_id, meta.module_id);
}

std::size_t Synchronizer::push_new_image_to_queue(CommonFrame meta)
{
  // Initialize the module mask to 1 for n_modules the least significant bits.
  auto mask = new_image_mask;
  mask.reset(meta.module_id % n_modules);
  cache.emplace(meta.image_id, std::make_pair(mask, meta));
  if (is_queue_too_long()) {
    cache.erase(cache.begin());
    return 1;
  }
  return 0;
}

size_t Synchronizer::update_module_mask_for_image(image_id id, size_t module_id)
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

CommonFrame Synchronizer::pop_next_full_image()
{
  if (cache.empty()) return CommonFrame{INVALID_IMAGE_ID, 0, 0};
  if (auto data = cache.begin()->second; data.first == 0) {
    cache.erase(cache.begin());
    return data.second;
  }
  return CommonFrame{INVALID_IMAGE_ID, 0, 0};
}

Synchronizer::modules_mask& Synchronizer::get_modules_mask_for_image(image_id id)
{
  return cache.find(id)->second.first;
}

bool Synchronizer::is_queue_too_long() const
{
  return cache.size() > n_images_buffer;
}

bool Synchronizer::is_new_image(image_id id) const
{
  return cache.find(id) == cache.end();
}

Synchronizer::modules_mask Synchronizer::set_all(int n_modules)
{
  modules_mask m;
  for (int i : std::views::iota(0, n_modules))
    m.set(i);
  return m;
}
