/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <chrono>

namespace sbr {

struct replay_settings
{
  std::size_t start_image_id;
  std::optional<std::size_t> end_image_id;
  std::optional<std::chrono::milliseconds> delay;
};

} // namespace std_driver
