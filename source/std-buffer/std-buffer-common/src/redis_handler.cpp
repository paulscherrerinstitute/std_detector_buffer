/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>

#include <fmt/core.h>
#include <range/v3/all.hpp>

#include "redis_handler.hpp"

using namespace std::chrono_literals;

namespace sbc {

RedisHandler::RedisHandler(std::string detector_name, const std::string& address)
    : prefix(std::move(detector_name) + ":")
    , redis(address)
{
  if (redis.ping() != "PONG")
    throw std::runtime_error(fmt::format("Connection to Redis API on address={} failed.", address));
}

void RedisHandler::send(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta)
{
  std::string meta_buffer_send;
  meta.SerializeToString(&meta_buffer_send);

  // TODO: For now expiry is fixed to 48 hours - most likely should be improved
  redis.setex(prefix + std::to_string(image_id), 48h, meta_buffer_send);
}

bool RedisHandler::receive(uint64_t image_id, std_daq_protocol::BufferedMetadata& meta)
{
  if (auto meta_buffer_send = redis.get(prefix + std::to_string(image_id))) {
    meta.ParseFromString(*meta_buffer_send);
    return true;
  }
  else
    return false;
}

std::vector<uint64_t> RedisHandler::get_more_recent_image_ids(uint64_t start_id, uint64_t stop_id)
{
  std::vector<std::string> keys = get_image_ids();

  auto get_id = [](const auto& data) {
    std::size_t pos = data.find(":");
    return data.substr(pos + 1);
  };
  auto to_number = [](const auto& data) { return static_cast<uint64_t>(stoll(data)); };
  auto filter_ids_from_range = [=](auto c) { return c >= start_id && c < stop_id; };

  auto image_ids = keys | ::ranges::view::transform(get_id) | ::ranges::view::transform(to_number) |
                   ::ranges::view::filter(filter_ids_from_range) | ::ranges::to_vector;

  return ::ranges::action::sort(image_ids);
}

std::vector<std::string> RedisHandler::get_image_ids()
{
  std::vector<std::string> keys;
  auto cursor = 0ll;
  do
    cursor = redis.scan(cursor, prefix + "*", 1000ll, std::back_inserter(keys));
  while (cursor != 0);
  return keys;
}

} // namespace sbc
