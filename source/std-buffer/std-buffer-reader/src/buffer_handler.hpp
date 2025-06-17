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
  static std::size_t get_uncompressed_size(const std_daq_protocol::ImageMetadata& data);
  void read_single_image(std_daq_protocol::BufferedMetadata& buffered_meta);

  void loader_loop(std::stop_token stoken);
  void request_loader_if_needed(uint64_t image_id);
  [[nodiscard]] std::optional<std_daq_protocol::ImageMetadata> try_pop_image(uint64_t image_id);
  [[nodiscard]] std::optional<std::vector<std_daq_protocol::BufferedMetadata>> fetch_next_metadatas(
      uint64_t image_id) const;

  std::deque<std_daq_protocol::ImageMetadata> metadatas_;
  sbc::FileHandler file_handler_;
  sbc::RedisHandler& redis_;
  cb::Communicator& communicator_;

  std::mutex mtx_;
  std::condition_variable_any cv_;
  std::jthread loader_;
  std::atomic<bool> running_{false};
  std::atomic<bool> loader_active_{false};
  std::atomic<bool> no_more_data_{false};
  uint64_t loader_image_id_ = 0;
};
