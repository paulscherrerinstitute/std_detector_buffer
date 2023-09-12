/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
#define STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP

#include <chrono>
#include <utility>
#include <string>
#include <string_view>


namespace gf::send {

class SenderStatsCollector : public utils::stats::StatsCollector<SenderStatsCollector>
{
public:
  explicit SenderStatsCollector(std::string_view detector_name, int part)
      : utils::stats::StatsCollector<SenderStatsCollector>(detector_name)
      , image_part(part)
  {}

  [[nodiscard]] std::string additional_message()
  {
    auto outcome = fmt::format("image_part={},zmq_fails={}", image_part, zmq_fails);
    zmq_fails = 0;
    return outcome;
  }

  void processing_finished(std::size_t send_fails)
  {
    zmq_fails += send_fails;
    static_cast<utils::stats::StatsCollector<SenderStatsCollector>*>(this)->processing_finished();
  }

private:
  int image_part;
  std::size_t zmq_fails = 0;
};

} // namespace gf::send

#endif // STD_DETECTOR_BUFFER_SENDER_STATS_COLLECTOR_HPP
