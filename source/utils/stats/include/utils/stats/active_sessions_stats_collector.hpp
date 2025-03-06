/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ACTIVE_SESSIONS_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_ACTIVE_SESSIONS_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "timed_stats_collector.hpp"

namespace utils::stats {

class ActiveSessionStatsCollector final : public TimedStatsCollector
{
public:
  explicit ActiveSessionStatsCollector(std::string_view detector_name,
                                       std::chrono::seconds period,
                                       std::string_view suffix)
      : TimedStatsCollector(detector_name, period, suffix)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},n_active_connections={}",
                               TimedStatsCollector::additional_message(), n_active_connections);
    return outcome;
  }

  void update_stats(unsigned long active_connections) { n_active_connections = active_connections; }

private:
  unsigned long n_active_connections = 0;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_ACTIVE_SESSIONS_STATS_COLLECTOR_HPP
