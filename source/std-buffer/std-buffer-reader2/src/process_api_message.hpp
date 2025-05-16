/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include <nlohmann/json.hpp>

#include "replay_settings.hpp"

namespace sbr {

std::optional<nlohmann::json> parse_command(const std::string& msg) noexcept;
std::optional<replay_settings> process_start_request(const nlohmann::json& cmd) noexcept;

}
