/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "utils/stats/timed_stats_collector.hpp"

namespace gf::rec {

class ReceiverStatsCollector : public utils::stats::TimedStatsCollector
{
public:
  explicit ReceiverStatsCollector(std::string_view detector_name, std::chrono::seconds period)
      : utils::stats::TimedStatsCollector(detector_name, period)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    auto outcome = fmt::format("{},zmq_receive_fails={},images_missed={}",
                               TimedStatsCollector::additional_message(), zmq_fails, images_missed);
    images_missed = 0;
    zmq_fails = 0;
    return outcome;
  }

  void processing_finished(unsigned int receive_fails, unsigned long id = 0)
  {
    zmq_fails += receive_fails;
    if (prev_image_id + 1 != id) images_missed++;
    prev_image_id = id > 0 ? id : prev_image_id;
    static_cast<utils::stats::TimedStatsCollector*>(this)->processing_finished();
  }

private:
  unsigned long zmq_fails = 0;
  unsigned long prev_image_id = 0;
  unsigned long images_missed = 0;
};

} // namespace gf::rec

#endif // STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP
