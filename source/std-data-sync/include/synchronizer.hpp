/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"

struct ImageAndSync
{
  const CommonFrame meta;
  const uint32_t n_corrupted_images;
};

class Synchronizer
{
  const int n_modules;
  const size_t n_images_buffer;
  std::queue<uint64_t> image_id_queue;
  std::unordered_map<uint64_t, std::pair<uint64_t, CommonFrame>> meta_cache;

public:
  Synchronizer(int n_modules, int n_images_buffer);
  ImageAndSync process_image_metadata(const CommonFrame& meta);
};

#endif // STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
