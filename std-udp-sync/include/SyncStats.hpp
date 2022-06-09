#ifndef STD_DETECTOR_BUFFER_SYNC_STATS_HPP
#define STD_DETECTOR_BUFFER_SYNC_STATS_HPP

#include <chrono>
#include <string>
#include <formats.hpp>

class SyncStats {
    const std::string detector_name_;
    const size_t stats_time_;

    int image_counter_;
    int n_sync_lost_images_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    SyncStats(const std::string &detector_name,
                   const size_t stats_time);

    void record_stats(const uint32_t n_lost_pulses);
};

#endif // STD_DETECTOR_BUFFER_SYNC_STATS_HPP
