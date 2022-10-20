/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"
#include "buffer_utils.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <iostream>
#include <date/date.h>
#include <fmt/core.h>

#include "sync_config.hpp"

using namespace std;
using namespace chrono;
using namespace sync_config;


Synchronizer::Synchronizer(const int n_modules, const int n_images_buffer)
    : n_modules(n_modules), n_images_buffer(n_images_buffer), image_id_queue(), meta_cache()
{
}

ImageAndSync Synchronizer::process_image_metadata(const CommonFrame& meta)
{
  // First time we see this image_id.
  if (meta_cache.find(meta.image_id) == meta_cache.end()) {
    image_id_queue.push(meta.image_id);

    // Initialize the module mask to 1 for n_modules least significant bits.
    uint64_t modules_mask = ~(~0u << n_modules);
    meta_cache[meta.image_id] = {modules_mask, meta};

    // If the queue is too long, drop the oldest non-complete image_id.
    if (image_id_queue.size() > n_images_buffer) {
      auto const image_id = image_id_queue.front();

      image_id_queue.pop();
      meta_cache.erase(image_id);
    }
  }

  // meta_cache format: map<image_id, pair<modules_mask, metadata>>
  auto& modules_mask = meta_cache.find(meta.image_id)->second.first;

  // Has this module already arrived for this image_id?
  if ((modules_mask & (1u << meta.module_id)) == 0) {
    throw runtime_error(fmt::format("Received same module_id 2 times for image_id {}",
                                    meta.image_id));
  }

  // Clear bit in 'module_id' place.
  modules_mask &= ~(1UL << meta.module_id);

  // All modules arrived for this image.
  if (modules_mask == 0) {
    uint32_t n_corrupted_images = 0;

    // Empty all older images until we get to the completed one.
    while (!image_id_queue.empty() && image_id_queue.front() != meta.image_id) {
      auto const image_id = image_id_queue.front();

      image_id_queue.pop();
      meta_cache.erase(image_id);

      n_corrupted_images++;
    }

    if (image_id_queue.empty()) {
      throw runtime_error(fmt::format("No images to return. Impossible?!"));
    }

    const ImageAndSync result{meta_cache.find(meta.image_id)->second.second, n_corrupted_images};

    image_id_queue.pop();
    meta_cache.erase(meta.image_id);

    return result;
  }

  // No image has been completed with this module.
  return {{}, 0};
}
