/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>

#include "utils/stats_collector.hpp"

#include "synchronizer.hpp"

namespace gf::rec {

class ReceiverStatsCollector : public utils::StatsCollector<ReceiverStatsCollector>
{
public:
  explicit ReceiverStatsCollector(std::string_view detector_name, Synchronizer& sync)
      : utils::StatsCollector<ReceiverStatsCollector>("std_stream_receive_gf", detector_name)
      , synchronizer(sync)
  {}

  [[nodiscard]] std::string additional_message()
  {
    auto outcome = fmt::format("packets_discarded={},zmq_receive_fails={}",
                               synchronizer.get_dropped_packages(), zmq_fails);
    zmq_fails = 0;
    return outcome;
  }

  void processing_finished(unsigned int receive_fails)
  {
    zmq_fails += receive_fails;
    static_cast<utils::StatsCollector<ReceiverStatsCollector>*>(this)->processing_finished();
  }

private:
  Synchronizer& synchronizer;
  unsigned long zmq_fails = 0;
};

} // namespace gf::rec

#endif // STD_DETECTOR_BUFFER_RECEIVER_STATS_COLLECTOR_HPP
