/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP

#include <chrono>
#include <string_view>
#include <fmt/core.h>

namespace utils {

template <typename Derived> class StatsCollector
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit StatsCollector(std::string_view app_name_, std::string_view detector_name_)
      : app_name(app_name_)
      , detector_name(detector_name_)
  {}
  void processing_started() { processing_start = std::chrono::steady_clock::now(); }
  void processing_finished()
  {
    using namespace std::chrono_literals;
    const auto now = std::chrono::steady_clock::now();
    stats.first += now - processing_start;
    stats.second++;

    if (10s < now - last_flush_time) {
      fmt::print("{},detector={},average_process_time_ns={},processed_times={}{} {}\n", app_name,
                 detector_name, (stats.first / stats.second).count(), stats.second,
                 additional_info(), timestamp());
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

  std::string additional_info()
  {
    if (auto info = (static_cast<Derived&>(*this)).additional_message(); !info.empty())
      return "," + info;
    else
      return {};
  }

  std::string_view app_name;
  std::string_view detector_name;
  time_point processing_start{};
  time_point last_flush_time{std::chrono::steady_clock::now()};
  std::pair<std::chrono::nanoseconds, std::size_t> stats{};
};

} // namespace utils

#endif // STD_DETECTOR_BUFFER_UTILS_STATS_COLLECTOR_HPP
