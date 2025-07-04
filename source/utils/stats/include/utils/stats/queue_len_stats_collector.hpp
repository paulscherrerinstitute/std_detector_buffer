/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "sync_stats_collector.hpp"

namespace utils::stats {

class QueueStatsCollector final : public utils::stats::SyncStatsCollector
{
  using Parent = utils::stats::SyncStatsCollector;

public:
  explicit QueueStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : SyncStatsCollector(detector_name, period)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},queue={}", Parent::additional_message(), queue);
    queue = 0;
    return outcome;
  }

  void update_queue_length(std::size_t queue_len) { queue = std::max(queue, queue_len); }

private:
  std::size_t queue = 0;
};

} // namespace sdss
