/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include "core_buffer/communicator.hpp"
#include "utils/detector_config.hpp"
#include "utils/stats/active_sessions_stats_collector.hpp"

#include "state_manager.hpp"
#include "replay_settings.hpp"

namespace sbr {

class replayer : public std::enable_shared_from_this<replayer>
{
  std::shared_ptr<sbr::state_manager> manager;
  void* zmq_ctx;
  std::atomic<reader_state> state{reader_state::idle};
  utils::stats::ActiveSessionStatsCollector stats;
  cb::Communicator receiver;
  void* push_socket;
  void* driver_socket;
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  explicit replayer(std::shared_ptr<sbr::state_manager> sm,
                         const utils::DetectorConfig& config,
                         const std::string& stream_address);
  void init(std::chrono::seconds logging_period);
  void start(const replay_settings& settings);
  void control_reader(const replay_settings& settings) const;
  void forward_images();
};

} // namespace std_driver
