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

namespace {

std::string get_batch_suffix(uint64_t image_id)
{
  return ':' + std::to_string(image_id / 1000000) + "000000";
}

}

RedisHandler::RedisHandler(std::string detector_name, const std::string& address)
    : key_prefix("camera:" + std::move(detector_name) + ":images")
    , redis(address)
{
  if (redis.ping() != "PONG")
    throw std::runtime_error(fmt::format("Connection to Redis API on address={} failed.", address));
}

void RedisHandler::send(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta)
{
  const auto key = key_prefix + get_batch_suffix(image_id);
  const auto id = std::to_string(image_id);

  std::string meta_buffer_send;
  meta.SerializeToString(&meta_buffer_send);

  auto pipe = redis.pipeline();
  pipe.hset(key, id, meta_buffer_send);
  pipe.expire(key, std::chrono::hours(12));
  pipe.exec();
}

bool RedisHandler::receive(uint64_t , std_daq_protocol::BufferedMetadata& )
{
  return true;
}

std::optional<uint64_t> RedisHandler::read_last_saved_image_id()
{
  return std::nullopt;
}

} // namespace sbc
