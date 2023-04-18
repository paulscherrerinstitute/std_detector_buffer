/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <fmt/core.h>
#include <chrono>

#include "redis_sender.hpp"

using namespace std::chrono_literals;

namespace sbw {

RedisSender::RedisSender(const std::string& address)
    : redis(address)
{
  if (redis.ping() != "PONG")
    throw std::runtime_error(fmt::format("Connection to Redis API on address={} failed.", address));
}

void RedisSender::set(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta)
{
  std::string meta_buffer_send;
  meta.SerializeToString(&meta_buffer_send);

  // TODO: For now expiry is fixed to 48 hours - most likely should be improved
  // TODO: Redis should also have better handling of keys - currently 2 cameras with same image_id
  // will override each other
  redis.setex(std::to_string(image_id), 48h, meta_buffer_send);
}

} // namespace sbw
