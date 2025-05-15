/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <range/v3/all.hpp>

#include "utils/version.hpp"

namespace utils::log {

class logger
{
public:
  explicit logger(std::string_view name, std::string_view log_level)
  {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto log_pattern =
        fmt::format("[%m-%d %H:%M:%S.%e]%^[\033[33m{}\033[m][v{}][%^%l%$] %v", name, PROJECT_VER);
    console_sink->set_pattern(log_pattern);
    console_sink->set_color_mode(spdlog::color_mode::always);
    auto logger = std::make_shared<spdlog::logger>("global", console_sink);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(to_log_level(log_level));
    spdlog::set_level(to_log_level(log_level));
  }

private:
  static std::string to_lower(std::string_view s)
  {
    return s | ranges::views::transform([](unsigned char c) { return std::tolower(c); }) |
           ranges::to<std::string>();
  }

  static spdlog::level::level_enum to_log_level(std::string_view level)
  {
    static std::unordered_map<std::string, spdlog::level::level_enum> log_level_map = {
        {"trace", spdlog::level::trace},  {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},    {"warn", spdlog::level::warn},
        {"warning", spdlog::level::warn}, {"err", spdlog::level::err},
        {"error", spdlog::level::err},    {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};

    auto log_level = to_lower(level);
    if (auto it = log_level_map.find(log_level); it != log_level_map.end()) return it->second;
    throw std::invalid_argument("Invalid log level string: " + log_level);
  }
};

} // namespace utils::log
