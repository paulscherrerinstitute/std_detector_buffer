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
  const CommonFrame meta;
  const uint32_t n_corrupted_images;
};

inline constexpr CommonFrame NoImage = {INVALID_IMAGE_ID, 0, 0};
inline constexpr ImageAndSync NoImageSynchronized = {NoImage, 0};

class Synchronizer
{
  const int n_modules;
  const size_t n_images_buffer;
  std::deque<uint64_t> image_id_queue;
  // meta_cache format: map<image_id, pair<modules_mask, metadata>>
  std::unordered_map<uint64_t, std::pair<uint64_t, CommonFrame>> meta_cache;

public:
  Synchronizer(int n_modules, int n_images_buffer);
  ImageAndSync process_image_metadata(const CommonFrame& meta);

private:
  uint64_t& get_modules_mask(uint64_t image_id);
  void push_new_image_to_queue(CommonFrame meta);
  void drop_oldest_incomplete_image();
  uint32_t discard_stale_images(uint64_t image_id);
  void discard_image(uint64_t image_id);
  ImageAndSync get_full_image(uint64_t image_id);

  bool is_new_image(uint64_t image_id) const;
  bool is_queue_too_long() const;
  static bool did_all_modules_arrive(uint64_t modules_mask);
};

#endif // STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
