/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_utils.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <zmq.h>
#include <fmt/core.h>

#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

void* buffer_utils::connect_socket(void* ctx, const string& buffer_name, const int zmq_socket_type)
{
  auto socket = create_socket(ctx, zmq_socket_type);
  const string ipc_address = IPC_URL_BASE + buffer_name;

#ifdef DEBUG_OUTPUT
  cout << "[buffer_utils::connect_socket]";
  cout << " IPC address: " << ipc_address << endl;
#endif

  if (zmq_connect(socket, ipc_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_socket_type == ZMQ_SUB) {
    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
      throw runtime_error(zmq_strerror(errno));
    }
  }

  return socket;
}

void* buffer_utils::bind_socket(void* ctx, const string& buffer_name, const int zmq_socket_type)
{
  auto socket = create_socket(ctx, zmq_socket_type);
  const string ipc_address = IPC_URL_BASE + buffer_name;

#ifdef DEBUG_OUTPUT
  cout << "[buffer_utils::bind_socket]";
  cout << " IPC address: " << ipc_address << endl;
#endif

  if (zmq_bind(socket, ipc_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_socket_type == ZMQ_SUB) {
    if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
      throw runtime_error(zmq_strerror(errno));
    }
  }

  return socket;
}

void* buffer_utils::create_socket(void* ctx, const int zmq_socket_type)
{

  void* socket = zmq_socket(ctx, zmq_socket_type);
  if (socket == nullptr) {
    throw runtime_error(zmq_strerror(errno));
  }

  // SUB and PULL sockets are used for receiving, the rest for sending.
  if (zmq_socket_type == ZMQ_SUB || zmq_socket_type == ZMQ_PULL) {
    int rcvhwm = BUFFER_ZMQ_RCVHWM;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
      throw runtime_error(zmq_strerror(errno));
    }
  }
  else {
    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
      throw runtime_error(zmq_strerror(errno));
    }
  }

  int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  return socket;
}
