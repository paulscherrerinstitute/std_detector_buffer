#ifndef STD_DETECTOR_BUFFER_ZMQ_LIVE_SENDER_HPP
#define STD_DETECTOR_BUFFER_ZMQ_LIVE_SENDER_HPP

#include <string>
#include "formats.hpp"
#include "buffer_utils.hpp"

class ZmqLiveSender
{
  void* socket;

private:
  std::string _get_data_type_mapping(int dtype) const;

public:
  ZmqLiveSender(void* ctx, const std::string& stream_address);
  ~ZmqLiveSender();

  void send(const ImageMetadata& meta, const char* data, size_t image_n_bytes);
};

#endif // STD_DETECTOR_BUFFER_ZMQ_LIVE_SENDER_HPP
