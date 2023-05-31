/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>
#include <charconv>

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

  const auto id = std::to_string(image_id);
  // TODO: For now expiry is fixed to 48 hours - most likely should be improved
  redis.setex(prefix + id, 48h, meta_buffer_send);
  redis.set(prefix + "last", id);
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

std::optional<uint64_t> RedisHandler::read_last_saved_image_id()
{
  auto to_uint64 = [](std::string_view sv) -> std::optional<uint64_t> {
    if (uint64_t value; std::from_chars(sv.begin(), sv.end(), value).ec == std::errc{})
      return value;
    return std::nullopt;
  };

  if (auto id = redis.get(prefix + "last")) return to_uint64(id.value());
  return std::nullopt;
}

} // namespace sbc
