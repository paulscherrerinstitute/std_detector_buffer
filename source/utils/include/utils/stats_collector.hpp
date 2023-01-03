/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP

#include <chrono>
#include <string_view>

namespace utils {

template <typename Derived> class StatsCollector
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit StatsCollector(std::string_view detector_name)
      : name(detector_name)
  {}
  void processing_started() { processing_start = std::chrono::steady_clock::now(); }
  void processing_finished()
  {
    using namespace std::chrono_literals;
    const auto now = std::chrono::steady_clock::now();
    stats.first += now - processing_start;
    stats.second++;

    if (10s < now - last_flush_time) {
      fmt::print("std_live_stream,detector={},average_process_time_ns={},processed_times={} {}\n",
                 name, (stats.first / stats.second).count(), stats.second, timestamp());
      std::fflush(stdout);

      last_flush_time = now;
      stats = {0ns, 0};
    }
  }

private:
  static auto timestamp()
  {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
  }

  std::string_view name;
  time_point processing_start{};
  time_point last_flush_time{std::chrono::steady_clock::now()};
  std::pair<std::chrono::nanoseconds, std::size_t> stats{};
};

} // namespace utils

#endif // STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP
