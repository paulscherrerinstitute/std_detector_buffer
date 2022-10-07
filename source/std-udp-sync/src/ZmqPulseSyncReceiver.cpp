/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "ZmqPulseSyncReceiver.hpp"
#include "buffer_utils.hpp"
#include "buffer_config.hpp"

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
  string ipc_address = buffer_config::IPC_URL_BASE + detector_name + "-sync";

#ifdef DEBUG_OUTPUT
  cout << "[ZmqPulseSyncReceiver::ZmqPulseSyncReceiver]";
  cout << " IPC address: " << ipc_address << endl;
#endif

  void* socket = zmq_socket(ctx, ZMQ_PULL);

  const int sndhwm = buffer_config::BUFFER_ZMQ_RCVHWM;
  if (zmq_setsockopt(socket, ZMQ_RCVHWM, &sndhwm, sizeof(sndhwm)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_bind(socket, ipc_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }
}

ZmqPulseSyncReceiver::~ZmqPulseSyncReceiver()
{
  zmq_close(socket_);
}

ImageAndSync ZmqPulseSyncReceiver::get_next_image_id() const
{
  uint64_t ids[n_modules_];

  for (uint32_t i_sync = 0; i_sync < SYNC_RETRY_LIMIT; i_sync++) {
    bool modules_in_sync = true;
    for (int i = 0; i < n_modules_; i++) {

//      zmq_recv(sockets_[i], &ids[i], sizeof(uint64_t), 0);

      if (ids[0] != ids[i]) {
        modules_in_sync = false;
      }
    }

    if (modules_in_sync) {
#ifdef DEBUG_OUTPUT
      using namespace date;
      cout << "[" << std::chrono::system_clock::now() << "]";
      cout << " [ZmqPulseSyncReceiver::get_next_image_id]";
      cout << " Modules in sync (";
      cout << " pulse_id " << ids[0] << ").";
      cout << endl;
#endif
      return {ids[0], i_sync};
    }

#ifdef DEBUG_OUTPUT
    using namespace date;
    cout << "[" << std::chrono::system_clock::now() << "]";
    cout << " [ZmqPulseSyncReceiver::get_next_image_id]";
    cout << " Modules out of sync:" << endl;
    for (int i = 0; i < n_modules_; i++) {
      cout << " module" << i << ":" << ids[i];
    }
    cout << endl;
#endif
  }

  stringstream err_msg;
  err_msg << "[ZmqPulseSyncReceiver::get_next_image_id]";
  err_msg << " SYNC_RETRY_LIMIT exceeded. State:";
  for (int i = 0; i < n_modules_; i++) {
    err_msg << " module" << i << ":" << ids[i];
  }
  err_msg << endl;

  throw runtime_error(err_msg.str());
}
