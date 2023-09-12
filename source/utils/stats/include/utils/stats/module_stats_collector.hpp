/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP

#include "stats_collector.hpp"

namespace utils::stats {

class ModuleStatsCollector : public utils::stats::StatsCollector<ModuleStatsCollector>
{
public:
  explicit ModuleStatsCollector(std::string_view detector_name,
                                std::chrono::seconds period,
                                int module_id)
      : utils::stats::StatsCollector<ModuleStatsCollector>(detector_name, period)
      , module_id(module_id)
  {}

  [[nodiscard]] std::string additional_message() const
  {
    return fmt::format("module_id={}", module_id);
  }

private:
  int module_id;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP
