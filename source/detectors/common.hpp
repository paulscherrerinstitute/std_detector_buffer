#ifndef STD_DETECTOR_BUFFER_COMMON_HPP
#define STD_DETECTOR_BUFFER_COMMON_HPP

#include <cstdint>

#pragma pack(push)
#pragma pack(1)
struct CommonFrame {
  // 18 bytes
  uint64_t image_id;
  uint64_t n_missing_packets;
  uint16_t module_id;
};
#pragma pack(pop)

#endif // STD_DETECTOR_BUFFER_COMMON_HPP
