/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

namespace gf::send {

class StatsCollector
{
public:
  explicit StatsCollector(std::string_view detector_name, bool first_half)
      : name(detector_name)
      , is_first_half(first_half)
  {}
  void processing_started();
  void processing_finished();

private:
  std::string_view name;
  bool is_first_half;
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> last_flush_time =
      std::chrono::steady_clock::now();
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> processing_start;
  std::pair<std::chrono::nanoseconds, std::size_t> stats;
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
