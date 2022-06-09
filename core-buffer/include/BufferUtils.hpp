#ifndef STD_DETECTOR_BUFFER_BUFFER_UTILS_HPP
#define STD_DETECTOR_BUFFER_BUFFER_UTILS_HPP

#include <string>
#include <vector>
#include <ostream>

namespace BufferUtils {

struct DetectorConfig
{
  const std::string detector_name;
  const std::string detector_type;
  const int n_modules;
  const int bit_depth;
  const int image_height;
  const int image_width;
  const int start_udp_port;

  friend std::ostream& operator<<(std::ostream& os, DetectorConfig const& det_config)
  {
    return os << det_config.detector_name << ' ' << det_config.detector_type << ' '
              << det_config.n_modules << ' ' << det_config.bit_depth << ' '
              << det_config.image_height << ' ' << det_config.image_width << ' '
              << det_config.start_udp_port << ' ';
  }
};

void* bind_socket(void* ctx, const std::string& detector_name, const std::string& stream_name);
void* connect_socket(void* ctx, const std::string& detector_name, const std::string& stream_name);
DetectorConfig read_json_config(const std::string& filename);
} // namespace BufferUtils

#endif // STD_DETECTOR_BUFFER_BUFFER_UTILS_HPP
