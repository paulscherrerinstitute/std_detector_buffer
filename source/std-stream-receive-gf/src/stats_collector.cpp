/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "stats_collector.hpp"

#include <fmt/core.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace gf::rec {

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

void StatsCollector::processing_finished(unsigned int receive_fails)
{
  const auto now = steady_clock::now();
  stats.first += now - processing_start;
  stats.second++;
  zmq_fails += receive_fails;

  if (10s < now - last_flush_time) {
    fmt::print("std_stream_receive_gf,detector={},average_process_time_ns={},number_of_processed_"
               "packets={},packets_discarded={},zmq_receive_fails={} "
               "{}\n",
               name, (stats.first / stats.second).count(), stats.second,
               synchronizer.get_dropped_packages(), zmq_fails, timestamp());
    std::fflush(stdout);
    synchronizer.reset_dropped_packages();

    last_flush_time = now;
    zmq_fails = 0;
    stats = {0ns, 0};
  }
}

} // namespace gf::rec
