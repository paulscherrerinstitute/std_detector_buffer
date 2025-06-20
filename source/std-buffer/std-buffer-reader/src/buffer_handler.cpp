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

BufferHandler::~BufferHandler()
{
  running_ = false;
  cv_.notify_all();
  if (loader_.joinable()) loader_.request_stop();
  if (loader_.joinable()) loader_.join();
}

void BufferHandler::start_loader()
{
  running_ = true;
  loader_ = std::jthread([this](std::stop_token stoken) { loader_loop(stoken); });
}

void BufferHandler::stop_loader()
{
  running_ = false;
  cv_.notify_all();
  if (loader_.joinable()) loader_.request_stop();
}

std::optional<std_daq_protocol::ImageMetadata> BufferHandler::get_image(uint64_t image_id)
{
  std::unique_lock lock(mtx_);

  if (auto meta = try_pop_image(image_id)) return meta;

  request_loader_if_needed(image_id);
  cv_.wait(lock, [this] { return !metadatas_.empty() || no_more_data_; });

  if (auto meta = try_pop_image(image_id)) return meta;
  return std::nullopt;
}

std::size_t BufferHandler::get_uncompressed_size(const std_daq_protocol::ImageMetadata& data)
{
  return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
}

void BufferHandler::loader_loop(std::stop_token stoken)
{
  while (!stoken.stop_requested()) {
    {
      std::unique_lock lock(mtx_);
      cv_.wait(lock, [this] { return loader_active_ || !running_; });
      if (!running_) break;

      loader_active_ = false;
      metadatas_.clear();
      no_more_data_ = false;
    }

    auto metadatas_opt = fetch_next_metadatas(loader_image_id_);
    if (!metadatas_opt) {
      {
        std::lock_guard lock(mtx_);
        no_more_data_ = true;
        cv_.notify_all();
      }
      continue;
    }

    for (auto& buffered_meta : *metadatas_opt) {
      if (stoken.stop_requested()) break;
      read_single_image(buffered_meta);
    }
    {
      std::lock_guard lock(mtx_);
      cv_.notify_all();
    }
  }
}

std::optional<std_daq_protocol::ImageMetadata> BufferHandler::try_pop_image(uint64_t image_id)
{
  if (!metadatas_.empty() && metadatas_.front().image_id() >= image_id) {
    auto metadata = metadatas_.front();
    metadatas_.pop_front();
    return metadata;
  }
  return std::nullopt;
}

void BufferHandler::request_loader_if_needed(uint64_t image_id)
{
  if (!loader_active_ || loader_image_id_ != image_id) {
    loader_active_ = true;
    loader_image_id_ = image_id;
    cv_.notify_all();
  }
}

std::optional<std::vector<std_daq_protocol::BufferedMetadata>> BufferHandler::fetch_next_metadatas(
    uint64_t image_id) const
{
  return file_handler_.get_first_matching_root_id(image_id).and_then([this](const uint64_t id) {
    if (auto metadatas = redis_.get_metadatas_in_file_range(id); !metadatas.empty())
      return std::optional{std::move(metadatas)};
    return std::optional<std::vector<std_daq_protocol::BufferedMetadata>>{};
  });
}

void BufferHandler::read_single_image(std_daq_protocol::BufferedMetadata& buffered_meta)
{
  const auto size = get_uncompressed_size(buffered_meta.metadata());
  const auto image = buffered_meta.metadata().image_id();

  file_handler_.read(image, {communicator_.get_data(image), size}, buffered_meta.offset(),
                     buffered_meta.metadata().size());
  buffered_meta.mutable_metadata()->set_size(size);
  buffered_meta.mutable_metadata()->set_compression(std_daq_protocol::none);

  {
    std::lock_guard lock(mtx_);
    bool was_empty = metadatas_.empty();
    metadatas_.push_back(buffered_meta.metadata());
    if (was_empty) cv_.notify_all();
  }
}
