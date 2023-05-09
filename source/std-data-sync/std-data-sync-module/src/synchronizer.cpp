/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <stdexcept>
#include <ranges>

#include <zmq.h>

using namespace std;

Synchronizer::Synchronizer(int n_modules,
                           int n_images_buffer,
                           std::chrono::milliseconds image_life,
                           modules_mask mask)
    : n_modules(n_modules)
    , n_images_buffer(n_images_buffer)
    , new_image_mask(mask.none() ? set_all(n_modules) : mask)
    , image_lifetime(image_life)
{}

std::size_t Synchronizer::process_image_metadata(CommonFrame meta)
{
  drop_expired_images();
  if (is_new_image(meta.image_id))
    return push_new_image_to_queue(meta);
  else
    return update_module_mask_for_image(meta.image_id, meta.module_id);
}

std::size_t Synchronizer::push_new_image_to_queue(CommonFrame meta)
{
  if (is_queue_too_long()) erase_element(cache.begin());
  // Initialize the module mask to 1 for n_modules the least significant bits.
  auto mask = new_image_mask;
  mask.reset(meta.module_id % n_modules);
  auto [iterator, _] = cache.emplace(meta.image_id, std::make_pair(mask, meta));
  time_ordered_cache.emplace(std::chrono::steady_clock::now(), iterator);
  return is_queue_too_long();
}

size_t Synchronizer::update_module_mask_for_image(image_id id, size_t module_id)
{
  auto& mask = get_modules_mask_for_image(id);

  // Has this module already arrived for this image_id?
  if (!mask.test(module_id % n_modules)) {
    erase_element(cache.find(id));
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
    erase_element(cache.begin());
    return data.second;
  }
  return CommonFrame{INVALID_IMAGE_ID, 0, 0};
}

void Synchronizer::drop_expired_images()
{
  const auto deletion_time = std::chrono::steady_clock::now() - image_lifetime;
  const auto last = time_ordered_cache.lower_bound(deletion_time);

  for (auto i : std::ranges::subrange{time_ordered_cache.begin(), last})
    cache.erase(i.second);
  time_ordered_cache.erase(time_ordered_cache.begin(), last);
}

void Synchronizer::erase_element(cache_iterator it)
{
  time_ordered_cache.erase(std::find_if(time_ordered_cache.begin(), time_ordered_cache.end(),
                                        [&](auto data) { return data.second == it; }));
  cache.erase(it);
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
