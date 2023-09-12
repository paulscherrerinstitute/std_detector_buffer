/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_BASIC_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_BASIC_STATS_COLLECTOR_HPP

#include "stats_collector.hpp"

namespace utils::stats {

class BasicStatsCollector : public utils::stats::StatsCollector<BasicStatsCollector>
{
public:
  explicit BasicStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : utils::stats::StatsCollector<BasicStatsCollector>(detector_name, period)
  {}

  static std::string additional_message() { return {}; }
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_BASIC_STATS_COLLECTOR_HPP
