/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
#define STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP

#include <string_view>

#include "utils/detector_config.hpp"
#include "core_buffer/communicator.hpp"

#include "state_manager.hpp"

namespace std_driver {

using writer_id = int32_t;

struct run_settings
{
  std::string path;
  std::size_t n_images;
  writer_id writer;
};

class writer_driver : public std::enable_shared_from_this<writer_driver>
{
  std::shared_ptr<std_driver::state_manager> manager;
  void* zmq_ctx;
  cb::Communicator receiver;
  std::vector<void*> sender_sockets;
  std::vector<void*> receiver_sockets;
  std::atomic<driver_state> state{driver_state::idle};
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  explicit writer_driver(std::shared_ptr<std_driver::state_manager> sm,
                         const std::string& source_name,
                         utils::DetectorConfig config,
                         std::size_t number_of_writers);
  void start(const run_settings& settings);

private:
  void* bind_sender_socket(const std::string& stream_address);
  void* connect_to_socket(const std::string& stream_address);
  void send_create_file_requests(std::string_view base_path, writer_id id);
  void record_images(std::size_t n_images);
  void send_save_file_requests();
  bool are_all_files_created();
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
