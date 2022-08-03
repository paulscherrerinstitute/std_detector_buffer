#include "ZmqLiveSender.hpp"

#include <iostream>
#include <stdexcept>

#include <zmq.h>
#include <fmt/core.h>

#include "stream_config.hpp"
using namespace std;
using namespace stream_config;

ZmqLiveSender::ZmqLiveSender(void* ctx,
                             const std::string& det_name,
                             const std::string& stream_address)
{
  socket = zmq_socket(ctx, ZMQ_PUB);

  const int sndhwm = STREAM_ZMQ_SNDHWM;
  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_bind(socket, stream_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }
}

ZmqLiveSender::~ZmqLiveSender()
{
  zmq_close(socket);
}

std::string ZmqLiveSender::_get_data_type_mapping(const int dtype) const
{
  if (dtype == 1)
    return "uint8";
  else if (dtype == 2)
    return "uint16";
  else if (dtype == 4)
    return "uint32";
  else if (dtype == 8)
    return "uint64";
  return "unknown";
}

void ZmqLiveSender::send(const ImageMetadata& meta, const char* data, const size_t image_n_bytes)
{
  zmq_send(socket, &meta, sizeof(meta), ZMQ_SNDMORE);
  zmq_send(socket, data, image_n_bytes, 0);
}
