/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include <nlohmann/json.hpp>

#include "run_settings.hpp"

namespace sbr {

std::optional<nlohmann::json> parse_command(const std::string& msg) noexcept;
std::optional<run_settings> process_start_request(const nlohmann::json& command) noexcept;

}
