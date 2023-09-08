/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "utils/stats/sync_stats_collector.hpp"

namespace sdss {

class QueueStatsCollector : public utils::stats::SyncStatsCollector
{
  using Parent = utils::stats::SyncStatsCollector;

public:
  explicit QueueStatsCollector(const std::string& prog_name, std::string_view detector_name)
      : SyncStatsCollector(prog_name, detector_name)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},queue={}", Parent::additional_message(), queue);
    queue = 0;
    return outcome;
  }

  void processing_finished(unsigned int n_corrupted, std::size_t queue_len)
  {
    queue = std::max(queue, queue_len);
    Parent::processing_finished(n_corrupted);
  }

private:
  std::size_t queue = 0;
};

} // namespace sdss
#endif // STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP
