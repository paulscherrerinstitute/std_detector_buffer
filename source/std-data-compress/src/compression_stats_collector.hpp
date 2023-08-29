/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP

#include <string>
#include <string_view>

#include <fmt/core.h>

#include "utils/stats_collector.hpp"

class CompressionStatsCollector : public utils::StatsCollector<CompressionStatsCollector>
{
public:
  explicit CompressionStatsCollector(const std::string& prog_name,
                                     std::string_view detector_name,
                                     std::size_t image_size)
      : utils::StatsCollector<CompressionStatsCollector>(prog_name, detector_name)
      , image_size(image_size)
  {}

  [[nodiscard]] virtual std::string additional_message()
  {
    auto ratio = (double)compressed_size / ((double)compressed_images * (double)image_size);
    auto outcome = fmt::format("compressed_images={},ratio={}", compressed_images, ratio);
    compressed_images = 0;
    compressed_size = 0;
    return outcome;
  }

  void processing_finished(int compressed)
  {
    if (compressed > 0) {
      compressed_images++;
      compressed_size += compressed;
    }
    static_cast<utils::StatsCollector<CompressionStatsCollector>*>(this)->processing_finished();
  }

private:
  std::size_t image_size;
  std::size_t compressed_images = 0;
  std::size_t compressed_size = 0;
};

#endif // STD_DETECTOR_BUFFER_COMPRESSION_STATS_COLLECTOR_HPP
