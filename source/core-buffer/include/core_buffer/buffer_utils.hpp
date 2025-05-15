/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <zmq.h>

namespace buffer_utils {

void* bind_socket(void* ctx, const std::string& buffer_name, int zmq_socket_type = ZMQ_PUB);
void* connect_socket_ipc(void* ctx, const std::string& buffer_name, int zmq_socket_type = ZMQ_SUB);
void* connect_socket(void* ctx, const std::string& buffer_name, int zmq_socket_type = ZMQ_SUB);
void* create_socket(void* ctx, int zmq_socket_type);

} // namespace buffer_utils
