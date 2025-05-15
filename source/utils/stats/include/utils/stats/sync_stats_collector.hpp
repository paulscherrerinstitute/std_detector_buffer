/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "timed_stats_collector.hpp"

namespace utils::stats {

class SyncStatsCollector : public TimedStatsCollector
{
public:
  explicit SyncStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : TimedStatsCollector(detector_name, period)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},n_corrupted_images={}",
                               TimedStatsCollector::additional_message(), n_corrupted_images);
    n_corrupted_images = 0;
    return outcome;
  }

  void process(unsigned int n_corrupted)
  {
    n_corrupted_images += n_corrupted;
    static_cast<TimedStatsCollector*>(this)->process();
  }

private:
  unsigned long n_corrupted_images = 0;
};

} // namespace utils::stats
