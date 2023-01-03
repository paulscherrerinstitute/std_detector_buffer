/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include <utils/stats_collector.hpp>

namespace send {

class StatsCollector : public utils::StatsCollector<StatsCollector>
{
public:
  explicit StatsCollector(std::string_view detector_name)
      : utils::StatsCollector<StatsCollector>("std_live_stream", detector_name)
  {}

  static std::string additional_message() { return {}; }
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
