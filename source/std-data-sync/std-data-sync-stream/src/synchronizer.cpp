/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <stdexcept>

#include <fmt/core.h>

using namespace std;

Synchronizer::Synchronizer(int parts, int n_images_buffer)
    : n_parts(parts)
    , n_images_buffer(n_images_buffer)
    , image_id_queue()
    , meta_cache()
{}

ImageAndSync Synchronizer::process_image_metadata(image_id id)
{
  auto n_corrupted = 0u;
  if (is_new_image(id)) n_corrupted = push_new_image_to_queue(id);

  auto& count = meta_cache.find(id)->second;
  if (++count == n_parts)
    return get_full_image(id);
  else
    return {INVALID_IMAGE_ID, n_corrupted};
}

ImageAndSync Synchronizer::get_full_image(image_id id)
{
  const auto n_corrupted_images = discard_stale_images(id);

  if (image_id_queue.empty()) throw runtime_error(fmt::format("No images to return. Impossible?!"));

  image_id_queue.pop_front();
  meta_cache.erase(id);

  return {id, n_corrupted_images};
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

std::size_t Synchronizer::push_new_image_to_queue(image_id id)
{
  image_id_queue.push_back(id);
  // Initialize the module mask to 1 for n_modules the least significant bits.
  meta_cache[id] = 0;
  if (is_queue_too_long()) {
    drop_oldest_incomplete_image();
    return 1;
  }
  return 0;
}

void Synchronizer::drop_oldest_incomplete_image()
{
  auto const id = image_id_queue.front();
  image_id_queue.pop_front();
  meta_cache.erase(id);
}

bool Synchronizer::is_queue_too_long() const
{
  return image_id_queue.size() > n_images_buffer;
}

bool Synchronizer::is_new_image(image_id id) const
{
  return meta_cache.find(id) == meta_cache.end();
}
