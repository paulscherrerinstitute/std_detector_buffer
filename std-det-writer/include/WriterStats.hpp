#ifndef STD_DETECTOR_BUFFER_WRITER_STATS_HPP
#define STD_DETECTOR_BUFFER_WRITER_STATS_HPP

#include <cstddef>
#include <string>
#include <chrono>

#include "formats.hpp"

class WriterStats {
    const std::string detector_name_;

    uint32_t image_n_bytes_{};

    int image_counter_{};
    uint64_t total_bytes_{};

    uint32_t total_buffer_write_us_{};
    uint32_t max_buffer_write_us_{};
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    explicit WriterStats(std::string detector_name,
            const uint64_t image_n_bytes);
    void end_run();
    void start_image_write();
    void end_image_write();
};

#endif // STD_DETECTOR_BUFFER_WRITER_STATS_HPP
