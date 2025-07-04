/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "timed_stats_collector.hpp"

namespace utils::stats {

class ModuleStatsCollector : public TimedStatsCollector
{
public:
  explicit ModuleStatsCollector(std::string_view detector_name,
                                std::chrono::seconds period,
                                int module_id)
      : TimedStatsCollector(detector_name, period)
      , module_id(module_id)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    return fmt::format("module_id={},{}", module_id, TimedStatsCollector::additional_message());
  }

private:
  int module_id;
};

} // namespace utils::stats
