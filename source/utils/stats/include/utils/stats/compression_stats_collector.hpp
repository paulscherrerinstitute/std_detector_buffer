/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "timed_stats_collector.hpp"

namespace utils::stats {

class CompressionStatsCollector final : public TimedStatsCollector
{
public:
  explicit CompressionStatsCollector(std::string_view detector_name,
                                     std::chrono::seconds period,
                                     std::size_t image_size)
      : TimedStatsCollector(detector_name, period)
      , image_size(image_size)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    double ratio = calculate_ratio();
    auto outcome = fmt::format("{},compressed_images={},ratio={}",
                               TimedStatsCollector::additional_message(), compressed_images, ratio);
    compressed_images = 0;
    compressed_size = 0;
    return outcome;
  }

  [[nodiscard]] double calculate_ratio() const
  {
    if (compressed_images > 0)
      return (double)compressed_size / ((double)compressed_images * (double)image_size);
    return 0.0;
  }

  void process(int compressed)
  {
    if (compressed > 0) {
      compressed_images++;
      compressed_size += compressed;
    }
    static_cast<TimedStatsCollector*>(this)->process();
  }

private:
  std::size_t image_size;
  std::size_t compressed_images = 0;
  std::size_t compressed_size = 0;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
