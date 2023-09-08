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
  explicit BasicStatsCollector(std::string_view app_name, std::string_view detector_name)
      : utils::stats::StatsCollector<BasicStatsCollector>(app_name, detector_name)
  {}

  static std::string additional_message() { return {}; }
};

} // namespace utils

#endif // STD_DETECTOR_BUFFER_BASIC_STATS_COLLECTOR_HPP
