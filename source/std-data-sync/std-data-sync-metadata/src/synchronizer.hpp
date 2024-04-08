/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <cstddef>
#include <set>
#include <map>
#include <optional>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"
#include "std_buffer/image_metadata.pb.h"

class Synchronizer
{
  using image_id = uint64_t;
  const size_t n_images_buffer;

  std::set<image_id> received_images;
  std::map<image_id, std_daq_protocol::ImageMetadata> metadata_cache;

public:
  explicit Synchronizer(std::size_t n_images_buffer);
  bool add_metadata(const std_daq_protocol::ImageMetadata& metadata);
  void add_received_image(image_id id);
  std::optional<std_daq_protocol::ImageMetadata> get_next_received_image();

  [[nodiscard]] std::size_t get_queue_length() const { return metadata_cache.size(); }
};

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
