/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"
#include "utils/image_id.hpp"

struct ImageAndSync
{
  const uint64_t image_id;
  const uint32_t n_corrupted_images;
};

class Synchronizer
{
  const std::size_t n_parts;
  const size_t n_images_buffer;

  std::deque<utils::image_id> image_id_queue;
  std::unordered_map<utils::image_id, std::size_t> meta_cache;

public:
  Synchronizer(int n_parts, int n_images_buffer);
  ImageAndSync process_image_metadata(utils::image_id id);
  [[nodiscard]] std::size_t get_queue_length() const { return image_id_queue.size(); }

private:
  uint32_t discard_stale_images(utils::image_id id);
  ImageAndSync get_full_image(utils::image_id id);
  std::size_t push_new_image_to_queue(utils::image_id id);
  void drop_oldest_incomplete_image();

  bool is_new_image(utils::image_id id) const;
  bool is_queue_too_long() const;
};
