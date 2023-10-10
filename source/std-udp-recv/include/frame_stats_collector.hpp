/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP

#include "utils/stats/module_stats_collector.hpp"

class FrameStatsCollector : public utils::stats::ModuleStatsCollector
{
public:
  explicit FrameStatsCollector(std::string_view detector_name,
                               std::chrono::seconds period,
                               int module_id)
      : utils::stats::ModuleStatsCollector(detector_name, period, module_id)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},frames_counter={},n_corrupted_frames={},n_missed_packets={}",
                               ModuleStatsCollector::additional_message(), frames_counter,
                               n_corrupted_frames, n_missed_packets);

    frames_counter = 0;
    n_missed_packets = 0;
    n_corrupted_frames = 0;

    return outcome;
  }

  void process(std::size_t n_missing_packets)
  {
    if (n_missing_packets > 0) {
      n_missed_packets += n_missing_packets;
      n_corrupted_frames++;
    }
    frames_counter++;
    ModuleStatsCollector::process();
  }

private:
  std::size_t frames_counter{};
  std::size_t n_missed_packets{};
  std::size_t n_corrupted_frames{};
};

#endif // STD_DETECTOR_BUFFER_FRAME_STATS_COLLECTOR_HPP
