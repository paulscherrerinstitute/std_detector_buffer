/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
#define STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP

#include <string_view>

#include "utils/detector_config.hpp"
#include "core_buffer/communicator.hpp"
#include "std_buffer/writer_command.pb.h"
#include "utils/stats/timed_stats_collector.hpp"

#include "state_manager.hpp"
#include "run_settings.hpp"

namespace std_driver {

class writer_driver : public std::enable_shared_from_this<writer_driver>
{
  std::shared_ptr<std_driver::state_manager> manager;
  void* zmq_ctx;
  cb::Communicator receiver;
  std::vector<void*> sender_sockets;
  std::vector<void*> receiver_sockets;
  std::atomic<driver_state> state{driver_state::idle};
  utils::stats::TimedStatsCollector stats;
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  explicit writer_driver(std::shared_ptr<std_driver::state_manager> sm,
                         const std::string& source_name,
                         utils::DetectorConfig config,
                         std::size_t number_of_writers);
  void init();
  void start(const run_settings& settings);

private:
  void* bind_sender_socket(const std::string& stream_address);
  void* connect_to_socket(const std::string& stream_address);
  void send_create_file_requests(std::string_view base_path, writer_id id);
  void record_images(std::size_t n_images);
  void send_command_to_all_writers(const std_daq_protocol::WriterAction& action);
  void send_save_file_requests();
  bool did_all_writers_record_data();
  bool did_all_writers_acknowledge();
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
