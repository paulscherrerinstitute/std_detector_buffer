/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <fmt/core.h>
#include <md5.h>

#include "bsread.hpp"
#include "map_dtype_to_stream_type.hpp"

namespace utils::stream {

std::tuple<std::string, std::string> prepare_bsread_headers(
    const std_daq_protocol::ImageMetadata& meta)
{

  auto data_header =
      fmt::format(R"({{"htype":"bsr_d-1.1","channels":[{{"name":"{}","shape":[{},{}])"
                  R"(,"type":"{}","compression":"bitshuffle_lz4"}}]}})",
                  meta.pco().bsread_name(), meta.width(), meta.height(),
                  map_dtype_to_stream_type(meta.dtype()));

  MD5 digest;
  auto encoded_data_header = data_header.c_str();
  digest.add(encoded_data_header, data_header.length());

  auto main_header = fmt::format(
      R"({{"htype":"bsr_m-1.1","pulse_id":{},"global_timestamp":{{"sec":{},"ns":{}}},"hash":"{}"}})",
      meta.image_id(), meta.pco().global_timestamp_sec(), meta.pco().global_timestamp_ns(),
      digest.getHash());

  return {main_header, data_header};
}

} // namespace utils::stream
