/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <string_view>

namespace std_driver {

enum class driver_state
{
  idle,
  started,
  creating_file,
  file_created,
  waiting_for_first_image,
  recording,
  saving_file,
  file_saved,
  stop,
  error,
  num_states
};

constexpr std::array<std::string_view, static_cast<size_t>(driver_state::num_states)> state_names{
    "idle",      "started",     "creating_file", "file_created", "waiting_for_first_image",
    "recording", "saving_file", "file_saved",    "stop",         "error"};

constexpr std::string_view to_string(driver_state state)
{
  auto index = static_cast<size_t>(state);
  return index < state_names.size() ? state_names[index] : "unknown";
}

} // namespace std_driver
