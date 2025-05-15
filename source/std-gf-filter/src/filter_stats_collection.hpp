/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "utils/stats/timed_stats_collector.hpp"

class FilterStatsCollector : public utils::stats::TimedStatsCollector
{
  using Parent = utils::stats::TimedStatsCollector;

public:
  explicit FilterStatsCollector(std::string_view detector_name,
                                std::chrono::seconds period,
                                std::string_view source)
      : utils::stats::TimedStatsCollector(detector_name, period, source)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome =
        fmt::format("{},forwarded_images={}", Parent::additional_message(), forwarded_images);
    reset_stats();
    return outcome;
  }

  void forward() { forwarded_images++; }

private:
  void reset_stats() { forwarded_images = 0; }
  unsigned forwarded_images = 0;
};
