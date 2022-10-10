/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_COMMUNICATOR_CONFIG_HPP
#define STD_DETECTOR_BUFFER_COMMUNICATOR_CONFIG_HPP

namespace cb {

constexpr inline int CONN_TYPE_BIND = 0;
constexpr inline int CONN_TYPE_CONNECT = 1;

struct CommunicatorConfig
{
  void* zmq_ctx;
  const int connection_type;
  const int zmq_socket_type;
};
} // namespace cb

#endif // STD_DETECTOR_BUFFER_RAM_BUFFER_CONFIG_HPP
