/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "stats_collector.hpp"

#include <fmt/core.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace gf::sdc {

namespace {
auto timestamp()
{
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

StatsCollector::StatsCollector(std::string name, int quadrant_id)
    : converter_name(std::move(name))
    , quadrant(quadrant_id)
{}

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
    fmt::print("std_data_convert_gf,converter_name={},quadrant_id={},"
               "average_process_time_ns={},number_of_processed_packets={} "
               "{}\n",
               converter_name, quadrant, (stats.first / stats.second).count(), stats.second,
               timestamp());

    last_flush_time = now;
    stats = {0ns, 0};
  }
}

} // namespace gf::sdc
