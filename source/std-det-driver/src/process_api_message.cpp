/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "process_api_message.hpp"

using namespace nlohmann;

namespace std_driver {

std::optional<json> parse_command(const std::string& msg) noexcept
{
  try {
    return json::parse(msg);
  }
  catch (const json::exception& e) {
    return std::nullopt;
  }
}

std::optional<run_settings> process_start_request(const nlohmann::json& command) noexcept
{
  if (command.value("command", "") == "start" && command.contains("path"))
    return run_settings{command["path"], command.value("n_image", 16777215ul),
                        command.value("writer_id", 0), command.value("start_id", 0ul)};
  return std::nullopt;
}

} // namespace std_driver
