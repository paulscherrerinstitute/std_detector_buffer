/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <stdexcept>
#include <algorithm>

#include <zmq.h>
#include <fmt/core.h>

using namespace std;

Synchronizer::Synchronizer(const int n_modules, const int n_images_buffer)
    : n_modules(n_modules)
    , n_images_buffer(n_images_buffer)
    , image_id_queue()
    , meta_cache()
{}

ImageAndSync Synchronizer::process_image_metadata(const CommonFrame& meta)
{
  if (is_new_image(meta.image_id)) push_new_image_to_queue(meta);

  // meta_cache format: map<image_id, pair<modules_mask, metadata>>
  auto& modules_mask = get_modules_mask(meta.image_id);

  // Has this module already arrived for this image_id?
  if ((modules_mask & (1u << meta.module_id % n_modules)) == 0) {
    discard_image(meta.image_id);
    return {NoImage, 1};
  }

  // Clear bit in 'module_id' place.
  modules_mask &= ~(1UL << meta.module_id % n_modules);

  if (did_all_modules_arrive(modules_mask)) return get_full_image(meta.image_id);
  return NoImageSynchronized;
}

ImageAndSync Synchronizer::get_full_image(uint64_t image_id)
{
  uint32_t n_corrupted_images = discard_stale_images(image_id);

  if (image_id_queue.empty()) throw runtime_error(fmt::format("No images to return. Impossible?!"));

  const ImageAndSync result{meta_cache.find(image_id)->second.second, n_corrupted_images};

  image_id_queue.pop_front();
  meta_cache.erase(image_id);

  return result;
}

uint32_t Synchronizer::discard_stale_images(uint64_t image_id)
{
  uint32_t n_corrupted_images = 0;

  // Empty all older images until we get to the completed one.
  while (!image_id_queue.empty() && image_id_queue.front() != image_id) {
    meta_cache.erase(image_id_queue.front());
    image_id_queue.pop_front();
    n_corrupted_images++;
  }
  return n_corrupted_images;
}

void Synchronizer::discard_image(uint64_t image_id)
{
  const auto [first, last] = std::ranges::remove(image_id_queue, image_id);
  image_id_queue.erase(first, last);
  meta_cache.erase(image_id);
}

bool Synchronizer::did_all_modules_arrive(uint64_t modules_mask)
{
  return modules_mask == 0;
}

bool Synchronizer::is_new_image(uint64_t image_id) const
{
  return meta_cache.find(image_id) == meta_cache.end();
}

void Synchronizer::push_new_image_to_queue(CommonFrame meta)
{
  image_id_queue.push_back(meta.image_id);

  // Initialize the module mask to 1 for n_modules the least significant bits.
  uint64_t modules_mask = ~(~0u << n_modules);
  meta_cache[meta.image_id] = {modules_mask, meta};

  if (is_queue_too_long()) drop_oldest_incomplete_image();
}

void Synchronizer::drop_oldest_incomplete_image()
{
  auto const image_id = image_id_queue.front();

  image_id_queue.pop_front();
  meta_cache.erase(image_id);
}

bool Synchronizer::is_queue_too_long() const
{
  return image_id_queue.size() > n_images_buffer;
}

uint64_t& Synchronizer::get_modules_mask(uint64_t image_id)
{
  return meta_cache.find(image_id)->second.first;
}
