/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "utils/stats_collector.hpp"

namespace gf::send {

class SenderStatsCollector : public utils::StatsCollector<SenderStatsCollector>
{
public:
  explicit SenderStatsCollector(std::string_view detector_name, int part)
      : utils::StatsCollector<SenderStatsCollector>("std_stream_send_gf", detector_name)
      , image_part(part)
  {}

  [[nodiscard]] std::string additional_message() const
  {
    return fmt::format("image_part={}", image_part);
  }

private:
  int image_part;
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
