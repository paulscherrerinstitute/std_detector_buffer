/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_UTILS_FILTER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_UTILS_FILTER_STATS_COLLECTOR_HPP

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

#endif // STD_DETECTOR_BUFFER_UTILS_FILTER_STATS_COLLECTOR_HPP
