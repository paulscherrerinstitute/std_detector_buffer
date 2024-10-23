/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "meta_buffer.hpp"

namespace {
constexpr size_t min_history_needed_for_operation = 998;
}

meta_buffer::meta_buffer(const std::size_t slots)
    : buffer(slots - min_history_needed_for_operation, {{}, {}})
{}

void meta_buffer::push(std_daq_protocol::ImageMetadata meta)
{
  const auto now = std::chrono::steady_clock::now();
  buffer[end] = std::make_pair(now, std::move(meta));
  end = increment(end);
}

std::optional<std_daq_protocol::ImageMetadata> meta_buffer::pop()
{
  if (auto index = begin;
      begin == end || period < std::chrono::steady_clock::now() - buffer[begin].first)
  {
    begin = increment(begin);
    return std::move(buffer[index].second);
  }
  return std::nullopt;
}

std::size_t meta_buffer::increment(const std::size_t index) const
{
  return (index + 1) % buffer.size();
}
