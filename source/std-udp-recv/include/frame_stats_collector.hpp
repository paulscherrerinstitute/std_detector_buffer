/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP

#include "utils/stats/stats_collector.hpp"

class FrameStatsCollector : public utils::stats::StatsCollector<FrameStatsCollector>
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit FrameStatsCollector(std::string_view detector_name,
                               std::chrono::seconds period,
                               int module_id)
      : utils::stats::StatsCollector<FrameStatsCollector>(detector_name, period)
      , module_id(module_id)
  {}

  [[nodiscard]] std::string additional_message()
  {
    using namespace std::chrono;

    auto interval_ms_duration =
        duration_cast<milliseconds>(steady_clock::now() - stats_interval_start).count();
    // * 1000 because milliseconds, + 250 because of truncation.
    auto rep_rate = ((frames_counter * 1000) + 250) / interval_ms_duration;

    auto outcome = fmt::format(
        "module_id={},rep_rate={},frames_counter={},n_corrupted_frames={},n_missed_packets={}",
        module_id, rep_rate, frames_counter, n_corrupted_frames, n_missed_packets);

    frames_counter = 0;
    n_missed_packets = 0;
    n_corrupted_frames = 0;
    stats_interval_start = steady_clock::now();

    return outcome;
  }

  void record_stats(std::size_t n_missing_packets)
  {
    if (n_missing_packets > 0) {
      n_missed_packets += n_missing_packets;
      n_corrupted_frames++;
    }
    frames_counter++;
  }

private:
  const int module_id;
  std::size_t frames_counter{};
  std::size_t n_missed_packets{};
  std::size_t n_corrupted_frames{};
  time_point stats_interval_start;
};

#endif // STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP
