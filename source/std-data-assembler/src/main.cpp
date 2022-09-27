/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <fmt/core.h>

#include "buffer_utils.hpp"

namespace {
void check_number_of_arguments(int argc)
{
  if (argc != 2) {
    fmt::print("Usage: std_data_assembler [detector_json_filename] \n\n"
               "\tdetector_json_filename: detector config file path.\n");
    exit(-1);
  }
}
} // namespace

int main(int argc, char* argv[])
{
  check_number_of_arguments(argc);

  const auto config = buffer_utils::read_json_config(std::string(argv[1]));
  return config.n_modules;
}
