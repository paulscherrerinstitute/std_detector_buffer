/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_handler.hpp"

BufferHandler::BufferHandler(sbc::RedisHandler& redis,
                             std::string root_directory,
                             std::size_t type_size,
                             cb::Communicator& communicator)
    : file_handler_(std::move(root_directory), type_size)
    , redis_(redis)
    , communicator_(communicator)
{}

void BufferHandler::start_loader()
{
  running_ = true;
  loader_ = std::jthread([this] { loader_loop(); });
}

void BufferHandler::stop_loader()
{
  running_ = false;
  if (loader_.joinable()) loader_.join();
}

std::optional<std_daq_protocol::ImageMetadata> BufferHandler::get_image(uint64_t image_id)
{
  if (!metadatas_.empty() && metadatas_.front().image_id() >= image_id) {
    auto metadata = metadatas_.front();
    metadatas_.pop_front();
    return metadata;
  }
  if (auto id = file_handler_.get_first_matching_root_id(image_id)) {
    spdlog::info("requested if {}, root_id_found {}", image_id, *id);
    metadatas_.clear(); // it might be that the data in the buffer is stale
    spdlog::info("metadata cleared");

    auto buffered_metadatas = redis_.get_metadatas_in_file_range(*id);

    for (auto& buffered_meta : buffered_metadatas) {
      // todo check if size fits buffer
      const auto size = get_uncompressed_size(buffered_meta.metadata());
      const auto image = buffered_meta.metadata().image_id();
      spdlog::info("buffered image {}, being read type {}", image, (int) buffered_meta.metadata().dtype());

      file_handler_.read(image, {communicator_.get_data(image), size}, buffered_meta.offset(),
                         buffered_meta.metadata().size());

      buffered_meta.mutable_metadata()->set_size(size);
      buffered_meta.mutable_metadata()->set_compression(std_daq_protocol::none);

      metadatas_.push_back(buffered_meta.metadata());
    }
    auto metadata = metadatas_.front();
    metadatas_.pop_front();
    return metadata;
  }
  return std::nullopt;
}

std::size_t BufferHandler::get_uncompressed_size(const std_daq_protocol::ImageMetadata& data)
{
  return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
}

void BufferHandler::loader_loop() {}
