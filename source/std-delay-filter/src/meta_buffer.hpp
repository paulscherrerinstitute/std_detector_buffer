/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <chrono>
#include <optional>

#include "std_buffer/image_metadata.pb.h"

class meta_buffer
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

public:
  explicit meta_buffer(std::size_t slots, std::chrono::seconds period);
  void push(std_daq_protocol::ImageMetadata meta);
  std::optional<std_daq_protocol::ImageMetadata> pop();

private:
  std::size_t increment(std::size_t index) const;

  const std::chrono::seconds period_;
  std::vector<std::pair<time_point, std_daq_protocol::ImageMetadata>> buffer_;
  std::size_t begin_ = 0u;
  std::size_t end_ = 0u;
};
