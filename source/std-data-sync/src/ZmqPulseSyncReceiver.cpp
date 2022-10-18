/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "ZmqPulseSyncReceiver.hpp"
#include "buffer_utils.hpp"

#include <zmq.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <iostream>
#include <date/date.h>

#include "sync_config.hpp"

using namespace std;
using namespace chrono;
using namespace sync_config;

ZmqPulseSyncReceiver::ZmqPulseSyncReceiver(void* ctx,
                                           const string& detector_name,
                                           const int n_modules)
    : ctx_(ctx)
    , n_modules_(n_modules)
{
  socket_ = buffer_utils::bind_socket(ctx_, detector_name + "-sync", ZMQ_PULL);
}

ZmqPulseSyncReceiver::~ZmqPulseSyncReceiver()
{
  zmq_close(socket_);
}

ImageAndSync ZmqPulseSyncReceiver::get_next_image_id() const
{
  uint64_t id;
  zmq_recv(socket_, &id, sizeof(uint64_t), 0);
  return {id, 0};
}
