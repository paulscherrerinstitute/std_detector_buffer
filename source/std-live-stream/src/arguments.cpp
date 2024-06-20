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
  program->add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer - default \"image\"")
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

  sending_config send_config;

  auto& group = program->add_mutually_exclusive_group();
  group.add_argument("-f", "--forward")
      .help("forward all images")
      .flag()
      .action([&send_config](const auto&) { send_config.type = sending_config::forward; });
  group.add_argument("-p", "--periodic")
      .help("periodically send Y images every N Hz (format: Y:N)")
      .action([&send_config](const std::string& arg) {
        if (auto delim_pos = arg.find(':'); delim_pos == std::string::npos)
          throw std::runtime_error("Format should be Y:N for --periodic");
        else {

          send_config.type = sending_config::periodic;
          send_config.value.first = std::stoul(arg.substr(0, delim_pos));
          send_config.value.second = std::stoul(arg.substr(delim_pos + 1));
          if (send_config.value.second < 1 || send_config.value.second > 1000)
            throw std::runtime_error("Invalid Hz rate, valid range [1-100] Hz");
        }
      });
  group.add_argument("-b", "--batch")
      .help("send Y images every N images (format: Y:N)")
      .action([&send_config](const std::string& arg) {
        if (auto delim_pos = arg.find(':'); delim_pos == std::string::npos)
          throw std::runtime_error("Format should be Y:N for --batch");
        else {
          send_config.type = sending_config::batch;
          send_config.value.first = std::stoul(arg.substr(0, delim_pos));
          send_config.value.second = std::stoul(arg.substr(delim_pos + 1));
        }
      });

  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"), program->get("--source_suffix"), send_config,
          program->get<stream_type>("--type")};
}

} // namespace ls
