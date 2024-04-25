/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
#define STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP

#include "state_manager.hpp"

namespace std_driver {

class writer_driver : public std::enable_shared_from_this<writer_driver>
{
  std_driver::state_manager& manager;

public:
  explicit writer_driver(std_driver::state_manager& sm);
  void start();
};

} // namespace std_driver

#endif // STD_DETECTOR_BUFFER_WRITER_DRIVER_HPP
