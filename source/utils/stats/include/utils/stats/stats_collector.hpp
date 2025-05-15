/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <string_view>

#include <spdlog/spdlog.h>

namespace utils::stats {

template <typename Derived> class StatsCollector
{
protected:
  ~StatsCollector() = default;

private:
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit StatsCollector(std::string_view detector_name_, std::chrono::seconds period_)
      : detector_name(detector_name_)
      , period(period_)
  {}

  void print_stats()
  {
    using namespace std::chrono_literals;
    const auto now = std::chrono::steady_clock::now();

    if (period < now - last_update_time) {
      spdlog::info("detector={}{} {}", detector_name, additional_info(), timestamp());
      last_update_time = now;
    }
  }

private:
  static auto timestamp()
  {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
  }

  std::string additional_info()
  {
    if (auto info = (static_cast<Derived&>(*this)).additional_message(); !info.empty())
      return "," + info;
    else
      return {};
  }

  std::string detector_name;
  std::chrono::seconds period;
  time_point last_update_time{std::chrono::steady_clock::now()};
};

} // namespace utils::stats
