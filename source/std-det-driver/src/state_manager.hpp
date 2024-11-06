/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_STATE_MANAGER_HPP
#define STD_DETECTOR_BUFFER_STATE_MANAGER_HPP

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <algorithm>
#include <chrono>

#include <spdlog/spdlog.h>

#include "driver_state.hpp"

namespace std_driver {

class state_manager
{
  std::atomic<driver_state> state{driver_state::idle};
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  void change_state(const driver_state newState)
  {
    {
      std::lock_guard lock(mutex);
      state = newState;
    }
    spdlog::debug("driver state changed: {}", to_string(newState));
    cv.notify_all();
  }

  [[nodiscard]] driver_state get_state() const { return state.load(); }

  bool is_recording() const
  {
    const auto current_state = state.load();
    return current_state == driver_state::recording ||
           current_state == driver_state::waiting_for_first_image;
  }

  template <typename... States> void wait_for_one_of_states(States... states) const
  {
    std::unique_lock lock(mutex);
    std::vector validStates = {states...};
    cv.wait(lock, [this, &validStates]() {
      return std::ranges::any_of(validStates, [this](auto s) { return state == s; });
    });
  }

  [[nodiscard]] driver_state wait_for_change_or_timeout(
      const std::chrono::milliseconds timeout) const
  {
    std::unique_lock lock(mutex);
    driver_state current_state = state.load();
    cv.wait_for(lock, timeout, [this, current_state]() { return state != current_state; });
    return state.load();
  }
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_STATE_MANAGER_HPP
