/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
#define STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "formats.hpp"

struct ImageAndSync
{
  const uint64_t image_id;
  const uint32_t n_lost_pulses;
};

class ZmqPulseSyncReceiver
{

  void* ctx_;
  const int n_modules_;

  std::vector<void*> sockets_;

public:
  ZmqPulseSyncReceiver(void* ctx, const std::string& detector_name, int n_modules);
  ~ZmqPulseSyncReceiver();

  ImageAndSync get_next_image_id() const;
};

#endif // STD_DETECTOR_BUFFER_ZMQ_PULSE_SYNC_RECEIVER_HPP
