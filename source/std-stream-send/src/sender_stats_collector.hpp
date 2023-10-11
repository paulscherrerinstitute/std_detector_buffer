/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "utils/stats/timed_stats_collector.hpp"

namespace gf::send {

class SenderStatsCollector : public utils::stats::TimedStatsCollector
{
public:
  explicit SenderStatsCollector(std::string_view detector_name,
                                std::chrono::seconds period,
                                int part)
      : TimedStatsCollector(detector_name, period)
      , image_part(part)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("image_part={},{},zmq_fails={}", image_part,
                               TimedStatsCollector::additional_message(), zmq_fails);
    zmq_fails = 0;
    return outcome;
  }

  void process(std::size_t send_fails)
  {
    zmq_fails += send_fails;
    static_cast<utils::stats::TimedStatsCollector*>(this)->process();
  }

private:
  int image_part;
  std::size_t zmq_fails = 0;
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
