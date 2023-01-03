/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "stats_collector.hpp"

#include <fmt/core.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace send {

namespace {
auto timestamp()
{
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

void StatsCollector::processing_started()
{
  processing_start = steady_clock::now();
}

void StatsCollector::processing_finished()
{
  const auto now = steady_clock::now();
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

} // namespace send
