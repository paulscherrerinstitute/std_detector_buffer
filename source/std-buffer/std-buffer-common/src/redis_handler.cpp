/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>
#include <charconv>

#include <fmt/core.h>
#include <range/v3/all.hpp>

#include "redis_handler.hpp"

using namespace std::chrono_literals;
using namespace sw::redis;

namespace sbc {
namespace {

uint64_t get_bucket_id(const uint64_t image_id)
{
  return (image_id / 1000) * 1000;
}

uint64_t increment_bucket(const uint64_t bucket_id)
{
  return bucket_id + 1000;
}

std::string get_bucket_suffix(const uint64_t image_id)
{
  return ':' + std::to_string(get_bucket_id(image_id));
}

} // namespace

RedisHandler::RedisHandler(std::string detector_name,
                           const std::string& address,
                           const std::size_t timeout)
    : key_prefix("camera:" + std::move(detector_name) + ":images")
    , ttl(std::chrono::hours(timeout))
    , redis(address)
{
  if (redis.ping() != "PONG")
    throw std::runtime_error(fmt::format("Connection to Redis API on address={} failed.", address));
}

void RedisHandler::send(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta)
{
  const auto key = key_prefix + get_bucket_suffix(image_id);
  const double score = image_id;  // required by Redis ZSET

  std::string meta_buffer_send;
  meta.SerializeToString(&meta_buffer_send);
  auto pipe = redis.pipeline();
  pipe.zadd(key, meta_buffer_send, score);
  pipe.expire(key, ttl);
  pipe.exec();
}

void RedisHandler::prepare_receiving(uint64_t from_id, std::optional<uint64_t> to_id)
{
  end_score = to_id ? static_cast<double>(*to_id) : std::numeric_limits<double>::max();
  bucket_id = get_bucket_id(from_id);
  receive_data_from_redis(from_id, from_id);
}

std::optional<std_daq_protocol::BufferedMetadata> RedisHandler::receive()
{
  if (proto_queue.empty()) return std::nullopt;

  std_daq_protocol::BufferedMetadata value = std::move(proto_queue.front());
  proto_queue.pop_front();
  if (proto_queue.empty()) receive_more();
  return value;
}

void RedisHandler::receive_more()
{
  bucket_id = increment_bucket(bucket_id);
  receive_data_from_redis(bucket_id, 0);
}

void RedisHandler::receive_data_from_redis(const uint64_t from_id, const double from_score)
{
  const auto key = key_prefix + get_bucket_suffix(from_id);

  std::vector<std::string> serialized_protos;
  redis.zrangebyscore(key, BoundedInterval<double>(from_score, end_score, BoundType::CLOSED),
                      std::back_inserter(serialized_protos));

  std_daq_protocol::BufferedMetadata proto;
  for (const auto& serialized : serialized_protos)
    if (proto.ParseFromString(serialized)) proto_queue.push_back(std::move(proto));
}

} // namespace sbc
