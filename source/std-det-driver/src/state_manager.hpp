/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

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
  std::atomic<unsigned int> images_processed{0};
  std::atomic<unsigned int> active_sessions;
  mutable std::mutex mutex;
  mutable std::condition_variable cv;

public:
  void change_state(const driver_state newState)
  {
    {
      std::lock_guard lock(mutex);
      state.store(newState, std::memory_order::release);
      if (newState == driver_state::idle) images_processed = 0;
    }
    spdlog::debug("driver state changed: {}", to_string(newState));
    cv.notify_all();
  }

  void update_image_count(const unsigned int sent_images)
  {
    std::lock_guard lock(mutex);
    images_processed = sent_images;
  }

  [[nodiscard]] driver_state get_state() const { return state.load(std::memory_order_acquire); }
  [[nodiscard]] unsigned int get_images_processed() const { return images_processed.load(); }
  [[nodiscard]] unsigned int get_active_sessions() const { return active_sessions.load(); }
  void add_active_session() { ++active_sessions; }
  void remove_active_session() { --active_sessions; }

  bool is_recording() const
  {
    const auto current_state = state.load(std::memory_order_acquire);
    return current_state == driver_state::recording ||
           current_state == driver_state::waiting_for_first_image ||
           current_state == driver_state::creating_file;
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
    driver_state current_state = state.load(std::memory_order_acquire);
    cv.wait_for(lock, timeout, [this, current_state]() { return state != current_state; });
    return state.load(std::memory_order_acquire);
  }
};

} // namespace std_driver
