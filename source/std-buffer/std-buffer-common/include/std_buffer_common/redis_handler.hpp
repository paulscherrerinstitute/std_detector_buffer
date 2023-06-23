/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_REDIS_HANDLER_HPP
#define STD_DETECTOR_BUFFER_REDIS_HANDLER_HPP

#include <string>
#include <optional>

#include <sw/redis++/redis++.h>

#include "std_buffer/buffered_metadata.pb.h"

namespace sbc {

class RedisHandler
{
public:
  explicit RedisHandler(std::string detector_name, const std::string& address);
  void send(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta);
  bool receive(uint64_t image_id, std_daq_protocol::BufferedMetadata& meta);
  std::optional<uint64_t> read_last_saved_image_id();

private:
  std::string prefix;
  sw::redis::Redis redis;
};

} // namespace sbc

#endif // STD_DETECTOR_BUFFER_REDIS_HANDLER_HPP
