/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "sync_stats.hpp"

#include <iostream>
#include <utility>

using namespace std;
using namespace chrono;

SyncStats::SyncStats(std::string detector_name, size_t stats_time)
    : detector_name_(std::move(detector_name))
    , stats_time_(stats_time * 1000)
{
  reset_counters();
}

void SyncStats::reset_counters()
{
  image_counter_ = 0;
  n_sync_lost_images_ = 0;
  stats_interval_start_ = steady_clock::now();
}

void SyncStats::record_stats(size_t n_lost_pulses)
{
  image_counter_++;
  n_sync_lost_images_ += n_lost_pulses;

  const auto time_passed = duration_cast<milliseconds>(steady_clock::now() - stats_interval_start_);

  if (time_passed >= stats_time_) {
    print_stats();
    reset_counters();
  }
}

void SyncStats::print_stats()
{
  auto interval_ms_duration =
      duration_cast<milliseconds>(steady_clock::now() - stats_interval_start_).count();
  // * 1000 because milliseconds, + 250 because of truncation.
  auto rep_rate = ((image_counter_ * 1000) + 250) / interval_ms_duration;
  uint64_t timestamp = time_point_cast<nanoseconds>(system_clock::now()).time_since_epoch().count();

  // Output in InfluxDB line protocol
  cout << "std_data_sync";
  cout << ",detector_name=" << detector_name_;
  cout << " ";
  cout << "n_processed_images=" << image_counter_ << "i";
  cout << ",n_sync_lost_images=" << n_sync_lost_images_ << "i";
  cout << ",repetition_rate=" << rep_rate << "i";
  cout << " ";
  cout << timestamp;
  cout << endl;
}
