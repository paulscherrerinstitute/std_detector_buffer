/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "process_api_message.hpp"

using namespace nlohmann;

namespace sbr {

std::optional<json> parse_command(const std::string& msg) noexcept
{
  try {
    return json::parse(msg);
  }
  catch (const json::exception& e) {
    return std::nullopt;
  }
}

std::optional<replay_settings> process_start_request(const nlohmann::json& cmd) noexcept
{
  if (cmd.value("command", "") == "start" && cmd.contains("start_image_id"))
    return replay_settings{
        .start_image_id = cmd.at("start_image_id").get<std::size_t>(),
        .end_image_id = cmd.contains("end_image_id")
                            ? std::make_optional(cmd.at("end_image_id").get<std::size_t>())
                            : std::nullopt,
        .delay =
            cmd.contains("delay")
                ? std::make_optional(std::chrono::milliseconds{cmd.at("delay").get<std::size_t>()})
                : std::nullopt};
  return std::nullopt;
}

} // namespace sbr
