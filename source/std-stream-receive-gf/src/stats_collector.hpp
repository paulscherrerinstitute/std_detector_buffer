/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "synchronizer.hpp"

namespace gf::rec {

class StatsCollector
{
public:
  explicit StatsCollector(std::string_view detector_name, Synchronizer& sync)
      : name(detector_name)
      , synchronizer(sync)
  {}
  void processing_started();
  void processing_finished(unsigned int receive_fails);

private:
  std::string_view name;
  Synchronizer& synchronizer;
  unsigned long zmq_fails = 0;
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> last_flush_time =
      std::chrono::steady_clock::now();
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> processing_start;
  std::pair<std::chrono::nanoseconds, std::size_t> stats;
};

} // namespace gf::rec

#endif // STD_DETECTOR_BUFFER_STATS_COLLECTOR_HPP
