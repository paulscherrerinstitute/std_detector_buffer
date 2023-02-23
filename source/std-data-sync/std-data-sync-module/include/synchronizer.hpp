/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP

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
  using modules_mask = uint64_t;
  using image_id = uint64_t;

  const int n_modules;
  const size_t n_images_buffer;
  std::deque<image_id> image_id_queue;
  std::unordered_map<image_id, modules_mask> meta_cache;

public:
  Synchronizer(int n_modules, int n_images_buffer);
  ImageAndSync process_image_metadata(image_id id, std::size_t module_id);

private:
  modules_mask& get_modules_mask_for_image(image_id id);
  uint32_t discard_stale_images(image_id id);
  ImageAndSync get_full_image(image_id id);
  void push_new_image_to_queue(image_id id);
  void drop_oldest_incomplete_image();
  void discard_image(image_id id);

  bool is_new_image(image_id id) const;
  bool is_queue_too_long() const;
  static bool did_all_modules_arrive(modules_mask mask);
};

#endif // STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
