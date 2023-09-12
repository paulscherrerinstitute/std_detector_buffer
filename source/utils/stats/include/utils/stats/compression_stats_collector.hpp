/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "stats_collector.hpp"

namespace utils::stats {

class CompressionStatsCollector : public StatsCollector<CompressionStatsCollector>
{
public:
  explicit CompressionStatsCollector(std::string_view detector_name,
                                     std::chrono::seconds period,
                                     std::size_t image_size)
      : StatsCollector<CompressionStatsCollector>(detector_name, period)
      , image_size(image_size)
  {}

  [[nodiscard]] virtual std::string additional_message()
  {
    double ratio = calculate_ratio();
    auto outcome = fmt::format("compressed_images={},ratio={}", compressed_images, ratio);
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

  void processing_finished(int compressed)
  {
    if (compressed > 0) {
      compressed_images++;
      compressed_size += compressed;
    }
    static_cast<StatsCollector<CompressionStatsCollector>*>(this)->processing_finished();
  }

private:
  std::size_t image_size;
  std::size_t compressed_images = 0;
  std::size_t compressed_size = 0;
};

} // namespace utils::stats

#endif // STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
