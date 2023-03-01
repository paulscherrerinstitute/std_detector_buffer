/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"

struct ImageAndSync
{
  const uint64_t image_id;
  const uint32_t n_corrupted_images;
};

inline constexpr ImageAndSync NoImageSynchronized = {INVALID_IMAGE_ID, 0};

class Synchronizer
{
  using image_id = uint64_t;

  const std::size_t n_parts;
  const size_t n_images_buffer;

  std::deque<image_id> image_id_queue;
  std::unordered_map<image_id, std::size_t> meta_cache;

public:
  Synchronizer(int n_parts, int n_images_buffer);
  ImageAndSync process_image_metadata(image_id id);

private:
  uint32_t discard_stale_images(image_id id);
  ImageAndSync get_full_image(image_id id);
  void push_new_image_to_queue(image_id id);
  void drop_oldest_incomplete_image();

  bool is_new_image(image_id id) const;
  bool is_queue_too_long() const;
};

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
