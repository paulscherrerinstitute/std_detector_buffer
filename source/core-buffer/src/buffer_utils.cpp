/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_utils.hpp"

#include <stdexcept>
#include <source_location>

#include <zmq.h>
#include <spdlog/spdlog.h>

#include "buffer_config.hpp"

using namespace buffer_config;

void* buffer_utils::connect_socket(void* ctx,
                                   const std::string& buffer_name,
                                   const int zmq_socket_type)
{
  auto socket = create_socket(ctx, zmq_socket_type);
  const std::string ipc_address = IPC_URL_BASE + buffer_name;

  spdlog::debug("{}: IPC address: {}", std::source_location::current().function_name(),
                ipc_address);

  if (zmq_connect(socket, ipc_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));
  return socket;
}

void* buffer_utils::bind_socket(void* ctx,
                                const std::string& buffer_name,
                                const int zmq_socket_type)
{
  auto socket = create_socket(ctx, zmq_socket_type);
  const std::string ipc_address = IPC_URL_BASE + buffer_name;

  spdlog::debug("{}: IPC address: {}", std::source_location::current().function_name(),
                ipc_address);

  if (zmq_bind(socket, ipc_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));
  return socket;
}

void* buffer_utils::create_socket(void* ctx, const int zmq_socket_type)
{

  void* socket = zmq_socket(ctx, zmq_socket_type);
  if (socket == nullptr) throw std::runtime_error(zmq_strerror(errno));

  if (zmq_socket_type == ZMQ_SUB && zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  // SUB and PULL sockets are used for receiving, the rest for sending.
  if (zmq_socket_type == ZMQ_SUB || zmq_socket_type == ZMQ_PULL) {
    int rcvhwm = BUFFER_ZMQ_RCVHWM;
    if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0)
      throw std::runtime_error(zmq_strerror(errno));

    const int timeout = 1000;
    if (zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
      throw std::runtime_error(zmq_strerror(errno));
  }
  else {
    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
      throw std::runtime_error(zmq_strerror(errno));
  }

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  return socket;
}
