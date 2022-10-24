/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <buffer_utils.hpp>
#include <ram_buffer.hpp>
#include <algorithm>

std::tuple<buffer_utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  if (argc != 3) {
    std::cout << std::endl;
    std::cout << "Usage: std_stream_send_gf [detector_json_filename] [stream_address]" << std::endl;
    std::cout << "\tdetector_json_filename: detector config file path." << std::endl;
    std::cout << "\tstream_address: address to bind the output stream." << std::endl;
    std::cout << std::endl;

    exit(-1);
  }
  return {buffer_utils::read_json_config(argv[1]), argv[2]};
}

int main(int argc, char* argv[])
{
  auto const [config, stream_address] = read_arguments(argc, argv);
  return config.detector_name == stream_address;
}
