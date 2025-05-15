/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string_view>

#include "utils/detector_config.hpp"
#include "std_buffer/writer_action.pb.h"
#include "utils/stats/active_sessions_stats_collector.hpp"

#include "state_manager.hpp"
#include "run_settings.hpp"

namespace sbr {

class writer_driver : public std::enable_shared_from_this<writer_driver>
{
  std::shared_ptr<sbr::state_manager> manager;
  void* zmq_ctx;
  void* sync_receive_socket;
  std::vector<void*> writer_send_sockets;
  std::vector<void*> writer_receive_sockets;
  std::atomic<reader_state> state{reader_state::idle};
  utils::stats::ActiveSessionStatsCollector stats;
  bool with_metadata_writer;
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  explicit writer_driver(std::shared_ptr<sbr::state_manager> sm,
                         const std::string& source_suffix,
                         const utils::DetectorConfig& config,
                         bool with_metadata_writer);
  void init(std::chrono::seconds logging_period);
  void start(const run_settings& settings);

private:
  void* bind_sender_socket(const std::string& stream_address) const;
  void* connect_to_socket(const std::string& stream_address) const;
  std::string generate_file_path_for_writer(std::string_view base_path, unsigned int index) const;
  std::size_t get_current_writer_index(unsigned int index) const;
  void* prepare_sync_receiver_socket(const std::string& source_name) const;
  void send_create_file_requests(std::string_view base_path, writer_id id) const;
  void record_images(std::size_t n_images);
  void send_command_to_all_writers(const std_daq_protocol::WriterAction& action);
  void send_save_file_requests();
  bool did_all_writers_record_data();
  bool did_all_writers_acknowledge();
};

} // namespace std_driver
