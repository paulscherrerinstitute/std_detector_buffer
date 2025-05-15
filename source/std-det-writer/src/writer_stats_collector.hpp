/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>

#include "utils/stats/stats_collector.hpp"

class WriterStatsCollector : public utils::stats::StatsCollector<WriterStatsCollector>
{
  using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

public:
  explicit WriterStatsCollector(std::string_view detector_name,
                                std::string source_suffix,
                                std::chrono::seconds period,
                                std::size_t image_bytes,
                                std::size_t id)
      : utils::stats::StatsCollector<WriterStatsCollector>(detector_name, period)
      , image_n_bytes(image_bytes)
      , source(std::move(source_suffix))
      , writer_id(id)
  {}

  [[nodiscard]] std::string additional_message()
  {
    using namespace std::chrono_literals;
    const auto [avg_buffer_write, avg_throughput] = calculate_averages();

    auto outcome =
        fmt::format("source={},id={},n_written_images={},avg_buffer_write_us={},max_buffer_"
                    "write_us={},avg_throughput={:.2f}",
                    source, writer_id, image_counter, avg_buffer_write, max_buffer_write.count(),
                    avg_throughput);

    image_counter = 0;
    total_bytes = 0;
    total_buffer_write = 0ns;
    max_buffer_write = 0ns;

    return outcome;
  }

  void start_image_write() { writing_start = std::chrono::steady_clock::now(); }

  void end_image_write()
  {
    using namespace std::chrono;
    image_counter++;
    total_bytes += image_n_bytes;

    auto write_duration = steady_clock::now() - writing_start;

    total_buffer_write += write_duration;
    max_buffer_write = max(max_buffer_write, write_duration);
  }

private:
  [[nodiscard]] std::tuple<std::size_t, double> calculate_averages() const
  {
    using namespace std::chrono;
    if (image_counter > 0) {
      const auto avg_buffer_write = total_buffer_write / image_counter;
      const auto avg_throughput =
          (double)total_bytes / total_buffer_write.count() * 1000000 / 1024 / 1024;
      return {avg_buffer_write.count(), avg_throughput};
    }
    else
      return {0, 0.0};
  }

  std::size_t image_n_bytes{};
  int image_counter{};
  std::size_t total_bytes{};
  std::chrono::nanoseconds total_buffer_write{};
  std::chrono::nanoseconds max_buffer_write{};
  time_point writing_start;
  std::string source;
  std::size_t writer_id;
};
