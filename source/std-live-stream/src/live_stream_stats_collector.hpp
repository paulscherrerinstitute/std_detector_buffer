/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP

#include "utils/stats/timed_stats_collector.hpp"
#include "arguments.hpp"

namespace ls {
class LiveStreamStatsCollector : public utils::stats::TimedStatsCollector
{
  using Parent = utils::stats::TimedStatsCollector;

public:
  explicit LiveStreamStatsCollector(const arguments& args)
      : TimedStatsCollector(
            args.config.detector_name, args.config.stats_collection_period, args.source_suffix_meta)
      , stype(args.type == ls::stream_type::array10 ? "array10" : "bsread")
      , port(args.stream_address.substr(args.stream_address.rfind(':') + 1))
  {}

  [[nodiscard]] std::string additional_message() override
  {
    return fmt::format("port={},type={},{}", port, stype, Parent::additional_message());
  }

private:
  std::string stype;
  std::string port;
};
} // namespace ls
#endif // STD_DETECTOR_BUFFER_LIVE_STREAM_STATS_COLLECTOR_HPP
