/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_PROCESS_API_MESSAGE_HPP
#define STD_DETECTOR_BUFFER_PROCESS_API_MESSAGE_HPP

#include <optional>

#include <nlohmann/json.hpp>

#include "run_settings.hpp"

namespace sbr {

std::optional<nlohmann::json> parse_command(const std::string& msg) noexcept;
std::optional<run_settings> process_start_request(const nlohmann::json& command) noexcept;

}
#endif // STD_DETECTOR_BUFFER_PROCESS_API_MESSAGE_HPP
