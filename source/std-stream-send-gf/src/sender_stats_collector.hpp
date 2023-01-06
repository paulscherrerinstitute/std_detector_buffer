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
  explicit SenderStatsCollector(std::string_view detector_name, bool first_half)
      : utils::StatsCollector<SenderStatsCollector>("std_stream_send_gf", detector_name)
      , is_first_half(first_half)
  {}

  [[nodiscard]] std::string additional_message() const
  {
    return fmt::format("is_first_half={}", is_first_half);
  }

private:
  bool is_first_half;
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
