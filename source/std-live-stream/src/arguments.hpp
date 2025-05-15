/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

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
  std::string source_suffix_meta;
  std::string source_suffix_image;
  stream_type type;
};

arguments read_arguments(int argc, char* argv[]);

} // namespace ls
