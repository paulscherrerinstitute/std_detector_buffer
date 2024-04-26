/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
#define STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP

#include "utils/detector_config.hpp"
#include "core_buffer/communicator.hpp"

#include "state_manager.hpp"

namespace std_driver {

class writer_driver : public std::enable_shared_from_this<writer_driver>
{
  std::shared_ptr<std_driver::state_manager> manager;
  void* zmq_ctx;
  cb::Communicator receiver;
  std::vector<void*> sender_sockets;

public:
  explicit writer_driver(std::shared_ptr<std_driver::state_manager> sm,
                         const std::string& source_name,
                         utils::DetectorConfig config,
                         std::size_t number_of_writers);
  void start();

private:
  void* bind_sender_socket(const std::string& stream_address);
  void send_create_file_requests();
  void record_images();
  void send_save_file_requests();
  bool are_all_files_created();
  bool are_all_files_saved();
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
