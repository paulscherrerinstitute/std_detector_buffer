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
  explicit TimedStatsCollector(std::string_view detector_name,
                               std::chrono::seconds period,
                               std::string_view suffix = "")
      : utils::stats::StatsCollector<TimedStatsCollector>(detector_name, period)
      , source_suffix(suffix)
  {}

  [[nodiscard]] virtual std::string additional_message()
  {
    const auto repetition_rate = calculate_repetition_rate();
    auto outcome = !source_suffix.empty() ? fmt::format("source={},", source_suffix) : "";
    outcome += fmt::format("processed_times={},repetition_rate_hz={:.2f}", processed_times,
                           repetition_rate);
    reset_stats();
    return outcome;
  }
  void process() { processed_times++; }

private:
  void reset_stats() { processed_times = 0; }
  double calculate_repetition_rate()
  {
    using namespace std::chrono;

    const auto now = steady_clock::now();
    const milliseconds period = duration_cast<milliseconds>(now - previous);
    previous = now;
    return processed_times * 1000.0 / static_cast<double>(period.count());
  }

  unsigned processed_times = 0;
  std::string source_suffix;
  time_point previous{std::chrono::steady_clock::now()};
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_UTILS_TIMED_STATS_COLLECTOR_HPP
