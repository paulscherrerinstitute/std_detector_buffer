/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_BSREAD_RECV_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_BSREAD_RECV_STATS_COLLECTOR_HPP

#include "utils/stats/timed_stats_collector.hpp"

class BsreadRecvStatsCollector : public utils::stats::TimedStatsCollector
{
  using Parent = utils::stats::TimedStatsCollector;

public:
  explicit BsreadRecvStatsCollector(std::string_view detector_name,
                                    std::string_view source,
                                    std::chrono::seconds period)
      : Parent(detector_name, period)
      , source_address(source)
  {}

  [[nodiscard]] std::string additional_message() override
  {
    return fmt::format("source={},{}", source_address, Parent::additional_message());
  }

private:
  std::string source_address;
};

#endif // STD_DETECTOR_BUFFER_BSREAD_RECV_STATS_COLLECTOR_HPP
