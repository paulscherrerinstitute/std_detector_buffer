/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "utils/stats_collector.hpp"

namespace gf::sync {

class SyncStatsCollector : public utils::StatsCollector<SyncStatsCollector>
{
public:
  explicit SyncStatsCollector(std::string_view detector_name)
      : utils::StatsCollector<SyncStatsCollector>("std_data_sync_module", detector_name)
  {}

  [[nodiscard]] std::string additional_message()
  {
    auto outcome = fmt::format("n_corrupted_images={}", n_corrupted_images);
    n_corrupted_images = 0;
    return outcome;
  }

  void processing_finished(unsigned int n_corrupted)
  {
    n_corrupted_images += n_corrupted;
    static_cast<utils::StatsCollector<SyncStatsCollector>*>(this)->processing_finished();
  }

private:
  unsigned long n_corrupted_images = 0;
};

} // namespace gf::sync

#endif // STD_DETECTOR_BUFFER_SYNC_STATS_COLLECTOR_HPP
