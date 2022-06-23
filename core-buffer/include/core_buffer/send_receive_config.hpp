#ifndef STD_DETECTOR_BUFFER_SEND_RECEIVE_CONFIG_HPP
#define STD_DETECTOR_BUFFER_SEND_RECEIVE_CONFIG_HPP

namespace cb {
struct SendReceiveConfig
{
  const std::string buffer_name;
  const size_t n_bytes_packet;
  const size_t n_bytes_data_packet;
  const size_t n_packets_frame;
  const size_t n_buffer_slots;
};
} // namespace cb

#endif // STD_DETECTOR_BUFFER_SEND_RECEIVE_CONFIG_HPP
