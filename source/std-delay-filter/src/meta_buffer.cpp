/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <spdlog/spdlog.h>

#include "meta_buffer.hpp"

namespace {
constexpr size_t min_history_needed_for_operation = 998;
}

meta_buffer::meta_buffer(const std::size_t slots, const std::chrono::seconds period)
    : period_(period)
    , buffer_(slots - min_history_needed_for_operation, {{}, {}})
{
  spdlog::info("Delaying buffer configured with period={}s, slots={}, buffer_size={}",
               period_.count(), slots, buffer_.size());
}

void meta_buffer::push(std_daq_protocol::ImageMetadata meta)
{
  const auto now = std::chrono::steady_clock::now();
  buffer_[end_] = std::make_pair(now, std::move(meta));
  end_ = increment(end_);
}

std::optional<std_daq_protocol::ImageMetadata> meta_buffer::pop()
{
  if (const auto index = begin_;
      index == end_ || period_ < std::chrono::steady_clock::now() - buffer_[index].first)
  {
    begin_ = increment(begin_);
    return std::move(buffer_[index].second);
  }
  return std::nullopt;
}

std::size_t meta_buffer::increment(const std::size_t index) const
{
  return (index + 1) % buffer_.size();
}
