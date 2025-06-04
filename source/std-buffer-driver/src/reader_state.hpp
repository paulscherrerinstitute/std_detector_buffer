/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <string_view>

namespace sbr {

enum class reader_state
{
  idle,
  replaying,
  finishing,
  finished,
  stop,
  error,
  num_states
};

constexpr std::array<std::string_view, static_cast<size_t>(reader_state::num_states)> state_names{
    "idle", "replaying", "finishing", "finished", "stop", "error"};

constexpr std::string_view to_string(reader_state state)
{
  auto index = static_cast<size_t>(state);
  return index < state_names.size() ? state_names[index] : "unknown";
}

} // namespace sbr
