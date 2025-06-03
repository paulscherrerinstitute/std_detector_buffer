/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>
#include <charconv>
#include <ranges>

#include <fmt/core.h>
#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include "redis_handler.hpp"


using namespace std::chrono_literals;
using namespace sw::redis;

namespace sbc {
namespace {

std::optional<uint64_t> parse_uint64(const std::string& s)
{
  try {
    size_t idx{};
    auto value = std::stoull(s, &idx);
    if (idx == s.size()) return value;
  }
  catch (...) {
  }
  return std::nullopt;
}

} // namespace

RedisHandler::RedisHandler(std::string detector_name,
                           const std::string& address,
                           const std::size_t timeout)
    : key_prefix("camera:" + std::move(detector_name) + ":")
    , ttl(std::chrono::hours(timeout))
    , redis(address)
{
  if (redis.ping() != "PONG")
    throw std::runtime_error(fmt::format("Connection to Redis API on address={} failed.", address));
}

void RedisHandler::send(const Meta& meta)
{
  auto image_id = meta.metadata().image_id();
  auto meta_key = make_meta_key(image_id);

  std::string blob;
  meta.SerializeToString(&blob);
  auto pipe = redis.pipeline();
  pipe.set(meta_key, blob);
  pipe.expire(meta_key, ttl);

  pipe.zadd(key_prefix + "ids", std::to_string(image_id), image_id);
  pipe.exec();
}

std::optional<std_daq_protocol::BufferedMetadata> RedisHandler::get_metadata(uint64_t image_id)
{
  auto key = make_meta_key(image_id);
  auto val = redis.get(key);
  if (!val) return std::nullopt;

  Meta meta;
  if (meta.ParseFromString(*val)) return meta;
  return std::nullopt;
}

std::vector<uint64_t> RedisHandler::get_image_ids_in_file_range(uint64_t file_base_id)
{
  const uint64_t end_id = file_base_id + 999;

  spdlog::info("requesting range {} {} ", file_base_id, end_id);
  std::vector<std::string> string_ids;
  redis.zrangebyscore(key_prefix + "ids",
                      sw::redis::BoundedInterval<double>(file_base_id, end_id, BoundType::CLOSED),
                      sw::redis::LimitOptions{}, std::back_inserter(string_ids));

  spdlog::info("received {}", string_ids.size());

  for (auto i = 0u; i < string_ids.size() && i < 10; ++i) {
    spdlog::info("ids {}: {}", i, string_ids[i]);
  }

  auto ids_view = string_ids |
                  std::views::transform([](const auto& id_str) { return parse_uint64(id_str); }) |
                  std::views::filter([](const auto& id) { return id.has_value(); }) |
                  std::views::transform([](const auto& id) { return *id; });

  return {ids_view.begin(), ids_view.end()};
}

std::vector<std_daq_protocol::BufferedMetadata> RedisHandler::get_metadatas_in_file_range(
    uint64_t file_base_id)
{
  auto ids = get_image_ids_in_file_range(file_base_id);
  std::vector<Meta> result;
  result.reserve(ids.size());
  for (auto id : ids) {
    if (auto meta = get_metadata(id)) result.push_back(std::move(*meta));
  }
  return result;
}

std::string RedisHandler::make_meta_key(uint64_t image_id) const
{
  return key_prefix + std::to_string(image_id);
}

} // namespace sbc
