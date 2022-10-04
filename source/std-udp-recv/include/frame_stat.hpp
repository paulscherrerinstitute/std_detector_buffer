/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_FRAME_STATS_HPP
#define STD_DETECTOR_BUFFER_FRAME_STATS_HPP

#include <cstddef>
#include <chrono>
#include <string>

#include "formats.hpp"

class FrameStats
{
  const std::string detector_name_;
  const int module_id_;
  const std::chrono::milliseconds stats_time_;

  int frames_counter_;
  int n_missed_packets_;
  int n_corrupted_frames_;
  std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

  void reset_counters();
  void print_stats();

public:
  FrameStats(std::string detector_name, uint16_t module_id, size_t stats_time);
  void record_stats(uint64_t n_missing_packets);
  void process_stats();
};

#endif // STD_DETECTOR_BUFFER_FRAME_STATS_HPP
