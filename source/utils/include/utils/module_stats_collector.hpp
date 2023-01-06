/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP

#include "stats_collector.hpp"

namespace utils {

class ModuleStatsCollector : public utils::StatsCollector<ModuleStatsCollector>
{
public:
  explicit ModuleStatsCollector(std::string_view app_name,
                                std::string_view detector_name,
                                int module_id)
      : utils::StatsCollector<ModuleStatsCollector>(app_name, detector_name)
      , module_id(module_id)
  {}

  [[nodiscard]] std::string additional_message() const
  {
    return fmt::format("module_id={}", module_id);
  }

private:
  int module_id;
};

} // namespace utils

#endif // STD_DETECTOR_BUFFER_MODULE_STATS_COLLECTOR_HPP
