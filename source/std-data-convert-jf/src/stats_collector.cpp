/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "stats_collector.hpp"

#include <fmt/core.h>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace sdc {

namespace {
auto timestamp()
{
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
} // namespace

StatsCollector::StatsCollector(sdc::Identifier converter_id)
    : id(std::move(converter_id))
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
    fmt::print("std_data_convert_jf,converter_id={},"
               "average_process_time_ns={},number_of_processed_packets={} "
               "{}\n",
               id.converter_name(), (stats.first / stats.second).count(), stats.second,
               timestamp());

    last_flush_time = now;
    stats = {0ns, 0};
  }
}

} // namespace sdc
