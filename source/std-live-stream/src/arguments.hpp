/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ARGUMENTS_HPP
#define STD_DETECTOR_BUFFER_ARGUMENTS_HPP

#include "utils/detector_config.hpp"
#include "utils/args.hpp"

namespace ls {

enum class stream_type
{
  bsread,
  array10
};

struct sending_config
{
  enum Type
  {
    forward,
    periodic,
    batch
  } type = forward;
  std::pair<size_t, size_t> value{0, 0};
};

struct arguments
{
  utils::DetectorConfig config;
  std::string stream_address;
  std::string source_suffix;
  sending_config send_config;
  stream_type type;
};

arguments read_arguments(int argc, char* argv[]);

} // namespace ls

#endif // STD_DETECTOR_BUFFER_ARGUMENTS_HPP
