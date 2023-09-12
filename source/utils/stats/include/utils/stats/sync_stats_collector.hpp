/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "stats_collector.hpp"

namespace utils::stats {

class SyncStatsCollector : public utils::stats::StatsCollector<SyncStatsCollector>
{
public:
  explicit SyncStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : utils::stats::StatsCollector<SyncStatsCollector>(detector_name, period)
  {}

  [[nodiscard]] virtual std::string additional_message()
  {
    auto outcome = fmt::format("n_corrupted_images={}", n_corrupted_images);
    n_corrupted_images = 0;
    return outcome;
  }

  void processing_finished(unsigned int n_corrupted)
  {
    n_corrupted_images += n_corrupted;
    static_cast<utils::stats::StatsCollector<SyncStatsCollector>*>(this)->processing_finished();
  }

private:
  unsigned long n_corrupted_images = 0;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP
