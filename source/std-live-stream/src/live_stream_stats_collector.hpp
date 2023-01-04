/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include <utils/stats_collector.hpp>

namespace send {

class LiveStreamStatsCollector : public utils::StatsCollector<LiveStreamStatsCollector>
{
public:
  explicit LiveStreamStatsCollector(std::string_view detector_name)
      : utils::StatsCollector<LiveStreamStatsCollector>("std_live_stream", detector_name)
  {}

  static std::string additional_message() { return {}; }
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
