/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "frame_stat.hpp"

#include <iostream>
#include <utility>

using namespace std;
using namespace chrono;

FrameStats::FrameStats(string detector_name, const uint16_t module_id, const size_t stats_time)
    : detector_name_(move(detector_name))
    , module_id_(module_id)
    , stats_time_(stats_time * 1000)
{
  reset_counters();
}

void FrameStats::reset_counters()
{
  frames_counter_ = 0;
  n_missed_packets_ = 0;
  n_corrupted_frames_ = 0;
  stats_interval_start_ = steady_clock::now();
}

void FrameStats::record_stats(const uint64_t n_missing_packets)
{
  if (n_missing_packets > 0) {
    n_missed_packets_ += n_missing_packets;
    n_corrupted_frames_++;
  }

  frames_counter_++;

  const auto time_passed = duration_cast<milliseconds>(steady_clock::now() - stats_interval_start_);

  if (time_passed >= stats_time_) {
    print_stats();
    reset_counters();
  }
}

void FrameStats::print_stats()
{
  auto interval_ms_duration =
      duration_cast<milliseconds>(steady_clock::now() - stats_interval_start_).count();
  // * 1000 because milliseconds, + 250 because of truncation.
  int rep_rate = ((frames_counter_ * 1000) + 250) / interval_ms_duration;
  uint64_t timestamp = time_point_cast<nanoseconds>(system_clock::now()).time_since_epoch().count();

  // Output in InfluxDB line protocol
  cout << "std_udp_recv";
  cout << ",detector_name=" << detector_name_;
  cout << ",module_id=" << module_id_;
  cout << " ";
  cout << "n_missed_packets=" << n_missed_packets_ << "i";
  cout << ",n_corrupted_frames=" << n_corrupted_frames_ << "i";
  cout << ",repetition_rate=" << rep_rate << "i";
  cout << " ";
  cout << timestamp;
  cout << endl;
}
