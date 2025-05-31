/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>
#include <thread>
#include <filesystem>
#include <iostream>

#include "core_buffer/communicator.hpp"
#include "std_buffer_common/redis_handler.hpp"
#include "std_buffer_common/file_handler.hpp"
#include "utils/utils.hpp"

class BufferHandler
{
public:
  BufferHandler(sbc::RedisHandler& redis,
                std::string root_directory,
                std::size_t type_size,
                cb::Communicator& communicator);

  void start_loader();
  void stop_loader();

  [[nodiscard]] std::optional<std_daq_protocol::ImageMetadata> get_image(uint64_t image_id);

private:
  void loader_loop();

  static std::size_t get_uncompressed_size(const std_daq_protocol::ImageMetadata& data);

  std::deque<std_daq_protocol::ImageMetadata> metadatas_;
  sbc::FileHandler file_handler_;
  sbc::RedisHandler& redis_;
  cb::Communicator& communicator_;
  std::jthread loader_;
  std::atomic<bool> running_{false};
};
