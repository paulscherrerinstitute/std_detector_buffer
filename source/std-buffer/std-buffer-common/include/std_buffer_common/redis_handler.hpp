/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <deque>

#include <sw/redis++/redis++.h>

#include "std_buffer/buffered_metadata.pb.h"

namespace sbc {

class RedisHandler
{
public:
  explicit RedisHandler(std::string detector_name, const std::string& address, std::size_t timeout);
  void send(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta);
  void prepare_receiving(uint64_t from_id, std::optional<uint64_t> to_id);
  std::optional<std_daq_protocol::BufferedMetadata> receive();

private:
  void receive_data_from_redis(uint64_t from_id, double from_score);
  void receive_more();

  uint64_t bucket_id = 0;
  double end_score = 0;
  std::string key_prefix;
  std::chrono::hours ttl;
  sw::redis::Redis redis;
  std::deque<std_daq_protocol::BufferedMetadata> proto_queue;
};

} // namespace sbc
