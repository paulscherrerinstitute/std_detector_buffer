/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "writer_driver.hpp"

#include <thread>
#include <chrono>

namespace std_driver {

writer_driver::writer_driver(std_driver::state_manager& sm)
    : manager(sm)
{}

void writer_driver::start()
{
  using namespace std::chrono_literals;
  auto self = shared_from_this();
  std::thread([self]() {
    self->manager.change_state(driver_state::creating_file);
    std::this_thread::sleep_for(500ms);
    self->manager.change_state(driver_state::file_created);
    std::this_thread::sleep_for(3ms);
    self->manager.change_state(driver_state::waiting_for_first_image);
    std::this_thread::sleep_for(2s);
    self->manager.change_state(driver_state::recording);
    std::this_thread::sleep_for(10s);
    self->manager.change_state(driver_state::saving_file);
    std::this_thread::sleep_for(40ms);
    self->manager.change_state(driver_state::file_saved);
  }).detach();
}

} // namespace std_driver
