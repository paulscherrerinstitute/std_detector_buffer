/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"
#include <spdlog/spdlog.h>

using namespace std;

Synchronizer::Synchronizer(std::size_t n_images_buffer)
    : n_images_buffer(n_images_buffer)
{}

bool Synchronizer::add_metadata(const std_daq_protocol::ImageMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(mutex_metadata);
  metadata_cache[metadata.image_id()] = metadata;
  if (metadata_cache.size() > n_images_buffer) {
    metadata_cache.erase(metadata_cache.begin());
    return true;
  }
  return false;
}

void Synchronizer::add_received_image(utils::image_id id)
{
  std::lock_guard<std::mutex> lock(mutex_images);
  received_images.insert(id);
  received_images.erase(received_images.begin(),
                        received_images.lower_bound(metadata_cache.begin()->first));
}

std::optional<std_daq_protocol::ImageMetadata> Synchronizer::get_next_received_image()
{
  std::scoped_lock lock(mutex_metadata, mutex_images);
  if (metadata_cache.empty() || received_images.empty()) return std::nullopt;
  if (metadata_cache.begin()->first == *(received_images.begin())) {
    auto data = metadata_cache.begin()->second;
    metadata_cache.erase(metadata_cache.begin());
    received_images.erase(received_images.begin());
    return data;
  }
  return std::nullopt;
}

std::size_t Synchronizer::get_queue_length() const
{
  std::lock_guard<std::mutex> lock(mutex_metadata);
  return metadata_cache.size();
}
