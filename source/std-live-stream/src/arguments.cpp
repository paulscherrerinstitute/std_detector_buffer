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
  program.add_argument("stream_address").help("address to bind the output stream");
  program.add_argument("-d", "--data_rate").help("rate in [Hz]").action([](const std::string& arg) {
    if (auto value = std::stoi(arg); value < 1 || value > 1000)
      throw std::runtime_error("Unsupported data_rate! [1-100 Hz] is valid");
    else
      return value;
  });
  program.add_argument("-s", "--source_suffix")
      .help("suffix for ipc source for ram_buffer - default \"image\"")
      .default_value("image"s);

  program.add_argument("-f", "--forward")
      .help("forward all images")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-t", "--type")
      .default_value(stream_type::array10)
      .help("choose the type: 'bsread' or 'array-1.0'")
      .action([](const std::string& value) {
        static const std::unordered_map<std::string, stream_type> type_map = {
            {"bsread", stream_type::bsread}, {"array-1.0", stream_type::array10}};
        if (auto it = type_map.find(value); it == type_map.end())
          throw std::runtime_error("Invalid choice for --type: " + value);
        else
          return it->second;
      });

  program = utils::parse_arguments(program, argc, argv);

  if (program.is_used("--data_rate") && program.is_used("--forward"))
    throw std::runtime_error(fmt::format("--data_rate and --forward can't be defined together"));

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address"), program.get("--source_suffix"),
          program["--forward"] == true ? 0 : program.get<std::size_t>("data_rate"),
          program.get<stream_type>("--type")};
}

} // namespace ls
