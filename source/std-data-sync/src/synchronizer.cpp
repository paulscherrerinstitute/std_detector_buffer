/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <stdexcept>
#include <algorithm>

#include <zmq.h>
#include <fmt/core.h>

using namespace std;

Synchronizer::Synchronizer(int n_parts, int n_images_buffer)
    : n_parts(n_parts)
    , n_images_buffer(n_images_buffer)
    , image_id_queue()
    , meta_cache()
{}

ImageAndSync Synchronizer::process_image_metadata(const CommonFrame& meta, std::size_t part_id)
{
  if (is_new_image(meta.image_id)) push_new_image_to_queue(meta);

  // meta_cache format: map<image_id, pair<modules_mask, metadata>>
  auto& mask = get_parts_mask_for_image(meta.image_id);

  // Has this module already arrived for this image_id?
  if ((mask & (1u << part_id % n_parts)) == 0) {
    discard_image(meta.image_id);
    return {NoImage, 1};
  }

  // Clear bit in 'part_id' place.
  mask &= ~(1UL << part_id % n_parts);

  if (did_all_modules_arrive(mask)) return get_full_image(meta.image_id);
  return NoImageSynchronized;
}

ImageAndSync Synchronizer::get_full_image(image_id id)
{
  const auto n_corrupted_images = discard_stale_images(id);

  if (image_id_queue.empty()) throw runtime_error(fmt::format("No images to return. Impossible?!"));

  const ImageAndSync result{meta_cache.find(id)->second.second, n_corrupted_images};

  image_id_queue.pop_front();
  meta_cache.erase(id);

  return result;
}

uint32_t Synchronizer::discard_stale_images(image_id id)
{
  uint32_t n_corrupted_images = 0;

  // Empty all older images until we get to the completed one.
  while (!image_id_queue.empty() && image_id_queue.front() != id) {
    meta_cache.erase(image_id_queue.front());
    image_id_queue.pop_front();
    n_corrupted_images++;
  }
  return n_corrupted_images;
}

void Synchronizer::discard_image(image_id id)
{
  const auto [first, last] = std::ranges::remove(image_id_queue, id);
  image_id_queue.erase(first, last);
  meta_cache.erase(id);
}

void Synchronizer::push_new_image_to_queue(CommonFrame meta)
{
  image_id_queue.push_back(meta.image_id);

  // Initialize the module mask to 1 for n_modules the least significant bits.
  uint64_t mask = ~(~0u << n_parts);
  meta_cache[meta.image_id] = {mask, meta};

  if (is_queue_too_long()) drop_oldest_incomplete_image();
}

void Synchronizer::drop_oldest_incomplete_image()
{
  auto const id = image_id_queue.front();
  image_id_queue.pop_front();
  meta_cache.erase(id);
}

Synchronizer::parts_mask& Synchronizer::get_parts_mask_for_image(image_id id)
{
  return meta_cache.find(id)->second.first;
}

bool Synchronizer::is_queue_too_long() const
{
  return image_id_queue.size() > n_images_buffer;
}

bool Synchronizer::did_all_modules_arrive(uint64_t modules_mask)
{
  return modules_mask == 0;
}

bool Synchronizer::is_new_image(image_id id) const
{
  return meta_cache.find(id) == meta_cache.end();
}
