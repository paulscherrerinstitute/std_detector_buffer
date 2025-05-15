/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

namespace cb {

constexpr inline int CONN_TYPE_BIND = 0;
constexpr inline int CONN_TYPE_CONNECT = 1;

struct CommunicatorConfig
{
  const std::string stream_name;
  void* zmq_ctx;
  const int connection_type;
  const int zmq_socket_type;
};
} // namespace cb
