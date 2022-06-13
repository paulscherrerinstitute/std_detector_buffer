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
  const size_t n_packets_per_frame_;
  const std::chrono::milliseconds stats_time_;

  int frames_counter_;
  int n_missed_packets_;
  int n_corrupted_frames_;
  std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

  void reset_counters();
  void print_stats();

public:
  FrameStats(std::string detector_name,
             const int module_id,
             const size_t n_packets_per_frame,
             const size_t stats_time);
  void record_stats(uint64_t n_missing_packets);
};

#endif // STD_DETECTOR_BUFFER_FRAME_STATS_HPP
