/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

namespace sdss {

class QueueStatsCollector : public utils::stats::SyncStatsCollector
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

  void process(unsigned int n_corrupted, std::size_t queue_len)
  {
    queue = std::max(queue, queue_len);
    Parent::process(n_corrupted);
  }

private:
  std::size_t queue = 0;
};

} // namespace sdss
#endif // STD_DETECTOR_BUFFER_QUEUE_LEN_STATS_COLLECTOR_HPP
