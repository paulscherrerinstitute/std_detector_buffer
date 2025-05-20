/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "arguments.hpp"

#include <string>

using namespace std::string_literals;

namespace ls {

arguments read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_live_stream");
  program->add_argument("stream_address").help("address to bind the output stream");
  program->add_argument("-sm", "--source_suffix_meta")
      .help("suffix for ipc - default \"image\"")
      .default_value("image"s);
  program->add_argument("-si", "--source_suffix_image")
      .help("suffix for shared memory sources for ram_buffer - default \"image\"")
      .default_value("image"s);
  program->add_argument("-t", "--type")
      .default_value(stream_type::array10)
      .help("choose the type: 'bsread' or 'array-1.0'")
      .action([](const std::string& arg) {
        static const std::unordered_map<std::string, stream_type> type_map = {
            {"bsread", stream_type::bsread}, {"array-1.0", stream_type::array10}};
        if (auto it = type_map.find(arg); it == type_map.end())
          throw std::runtime_error("Invalid choice for --type: " + arg);
        else
          return it->second;
      });
  program->add_argument("-s", "--socket")
      .default_value(socket_type::pub)
      .help("choose the type: 'pub' or 'push'")
      .action([](const std::string& arg) {
        static const std::unordered_map<std::string, socket_type> type_map = {
            {"pub", socket_type::pub}, {"push", socket_type::push}};
        if (auto it = type_map.find(arg); it == type_map.end())
          throw std::runtime_error("Invalid choice for --type: " + arg);
        else
          return it->second;
      });

  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"),
          program->get("--source_suffix_meta"),
          program->get("--source_suffix_image"),
          program->get<stream_type>("--type"),
          program->get<socket_type>("--socket")};
}

} // namespace ls
