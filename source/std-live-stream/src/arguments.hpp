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

struct arguments
{
  utils::DetectorConfig config;
  std::string stream_address;
  std::string source_suffix;
  utils::live_stream_config send_config;
  stream_type type;
};

arguments read_arguments(int argc, char* argv[]);

} // namespace ls

#endif // STD_DETECTOR_BUFFER_ARGUMENTS_HPP
