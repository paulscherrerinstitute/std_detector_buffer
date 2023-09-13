/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_UTILS_TIMED_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_UTILS_TIMED_STATS_COLLECTOR_HPP

#include "stats_collector.hpp"

namespace utils::stats {

class TimedStatsCollector : public utils::stats::StatsCollector<TimedStatsCollector>
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit TimedStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : utils::stats::StatsCollector<TimedStatsCollector>(detector_name, period)
  {}

  [[nodiscard]] virtual std::string additional_message()
  {
    auto outcome =
        fmt::format("average_process_time_ns={},max_process_time_ns={},processed_times={}",
                    (stats.first / std::max(stats.second, 1ul)).count(),
                    max_processing_time.count(), stats.second);
    reset_stats();
    return outcome;
  }
  void processing_started() { processing_start = std::chrono::steady_clock::now(); }
  void processing_finished() { update_stats(std::chrono::steady_clock::now()); }

private:
  void reset_stats()
  {
    using namespace std::chrono_literals;
    stats = {0ns, 0};
    max_processing_time = 0ns;
  }

  void update_stats(time_point now)
  {
    const auto processing_time = now - processing_start;
    max_processing_time = std::max(max_processing_time, processing_time);
    stats.first += processing_time;
    stats.second++;
  }

  time_point processing_start{};
  time_point last_update_time{std::chrono::steady_clock::now()};
  std::pair<std::chrono::nanoseconds, std::size_t> stats{};
  std::chrono::nanoseconds max_processing_time{};
};

template <typename TimedStatsCollector> class process_stats
{
public:
  explicit process_stats(TimedStatsCollector& c)
      : collector(c)
  {
    collector.processing_started();
  }
  ~process_stats() { collector.processing_finished(); }

private:
  TimedStatsCollector& collector;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_UTILS_TIMED_STATS_COLLECTOR_HPP
