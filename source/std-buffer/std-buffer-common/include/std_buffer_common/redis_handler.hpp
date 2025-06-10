/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string_view>
#include <optional>
#include <chrono>

#include <sw/redis++/redis++.h>

#include "std_buffer/buffered_metadata.pb.h"

namespace sbc {

class RedisHandler
{
  using Meta = std_daq_protocol::BufferedMetadata;

public:
  explicit RedisHandler(std::string_view detector_name,
                        const std::string& address,
                        std::size_t timeout);
  void send(const Meta& meta);
  [[nodiscard]] std::vector<Meta> get_metadatas_in_file_range(uint64_t file_base_id);

private:
  [[nodiscard]] std::vector<uint64_t> get_image_ids_in_file_range(uint64_t file_base_id);
  [[nodiscard]] std::optional<Meta> get_metadata(uint64_t image_id);
  [[nodiscard]] std::string make_meta_key(uint64_t image_id) const;

  std::string key_prefix;
  std::chrono::hours ttl;
  sw::redis::Redis redis;
};

} // namespace sbc
