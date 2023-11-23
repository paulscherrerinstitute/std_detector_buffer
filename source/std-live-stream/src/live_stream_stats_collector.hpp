/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP

#include "utils/stats/timed_stats_collector.hpp"
#include "arguments.hpp"

class LiveStreamStatsCollector : public utils::stats::TimedStatsCollector
{
  using Parent = utils::stats::TimedStatsCollector;

public:
  explicit LiveStreamStatsCollector(std::string_view detector_name,
                                    ls::stream_type type,
                                    std::chrono::seconds period)
      : TimedStatsCollector(detector_name, period)
      , stype(type == ls::stream_type::array10 ? "array10" : "bsread")
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("type={},{}", stype, Parent::additional_message());
    return outcome;
  }

private:
  std::string stype;
};

#endif // STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
