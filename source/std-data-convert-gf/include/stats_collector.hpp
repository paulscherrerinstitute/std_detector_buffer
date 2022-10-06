/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

namespace gf::sdc {

class StatsCollector
{
public:
  explicit StatsCollector(std::string name, int quadrant_id);
  void processing_started();
  void processing_finished();

private:
  std::string converter_name;
  int quadrant;
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> last_flush_time =
      std::chrono::steady_clock::now();
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> processing_start;
  std::pair<std::chrono::nanoseconds, std::size_t> stats;
};

} // namespace jf::sdc

#endif // STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
