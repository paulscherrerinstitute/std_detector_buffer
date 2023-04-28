/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_REDIS_SENDER_HPP
#define STD_DETECTOR_BUFFER_REDIS_SENDER_HPP

#include <string>

#include <sw/redis++/redis++.h>

#include "std_daq/buffered_metadata.pb.h"

namespace sbw {

class RedisSender
{
public:
  explicit RedisSender(std::string detector_name, const std::string& address);
  void set(uint64_t image_id, const std_daq_protocol::BufferedMetadata& meta);

private:
  std::string prefix;
  sw::redis::Redis redis;
};

} // namespace sbw

#endif // STD_DETECTOR_BUFFER_REDIS_SENDER_HPP
