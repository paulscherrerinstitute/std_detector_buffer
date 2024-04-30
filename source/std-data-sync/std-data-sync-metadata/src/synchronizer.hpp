/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <cstddef>
#include <set>
#include <map>
#include <mutex>
#include <optional>

#include "core_buffer/formats.hpp"
#include "detectors/common.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/image_id.hpp"

class Synchronizer
{
  const size_t n_images_buffer;

  std::set<utils::image_id> received_images;
  std::map<utils::image_id, std_daq_protocol::ImageMetadata> metadata_cache;
  mutable std::mutex mutex_metadata;
  mutable std::mutex mutex_images;

public:
  explicit Synchronizer(std::size_t n_images_buffer);
  bool add_metadata(const std_daq_protocol::ImageMetadata& metadata);
  void add_received_image(utils::image_id id);
  std::optional<std_daq_protocol::ImageMetadata> get_next_received_image();

  [[nodiscard]] std::size_t get_queue_length() const;
};

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
