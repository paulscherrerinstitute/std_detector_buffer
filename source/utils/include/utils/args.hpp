/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ARGS_HPP
#define STD_DETECTOR_BUFFER_ARGS_HPP

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

#include <argparse/argparse.hpp>

#include "utils/version.hpp"

namespace utils {

using parser = std::unique_ptr<argparse::ArgumentParser>;

inline auto create_parser(std::string program_name)
{
  auto prog = std::make_unique<argparse::ArgumentParser>(std::move(program_name), PROJECT_VER);
  prog->add_argument("detector_json_filename");
  return prog;
}

inline parser parse_arguments(parser program_parser, int argc, char* argv[])
{
  try {
    program_parser->parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program_parser;
    std::exit(1);
  }
  return program_parser;
}

} // namespace utils

#endif // STD_DETECTOR_BUFFER_ARGS_HPP
