/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_EIGER_HPP
#define STD_DETECTOR_BUFFER_EIGER_HPP

#include <cstdint>
#include <string>

#include "common.hpp"

#define IS_BOTTOM(n) ((n % 2 != 0) ? -1 : 1)

const std::string DETECTOR_TYPE = "eiger";

#define N_MODULES 1
#define BYTES_PER_PACKET 4144
#define DATA_BYTES_PER_PACKET 4096

#define MODULE_X_SIZE 512
#define MODULE_Y_SIZE 256
#define MODULE_N_PIXELS 131072
#define PIXEL_N_BYTES 2
#define GAP_X_MODULE_PIXELS 2
#define GAP_Y_MODULE_PIXELS 2
#define GAP_X_EIGERMOD_PIXELS 8
#define GAP_Y_EIGERMOD_PIXELS 36
#define EXTEND_X_PIXELS 3
#define EXTEND_Y_PIXELS 1

#pragma pack(push)
#pragma pack(1)
struct EGFrame
{
  // 18 bytes
  CommonFrame common;

  // 6 bytes
  uint16_t bit_depth;
  uint16_t pos_y;
  uint16_t pos_x;

  // 16 bytes
  uint32_t exptime;
  double bunchid;
  uint32_t debug;

  char __padding__[DET_FRAME_STRUCT_BYTES - 18 - 6 - 16];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(2)
struct EGUdpPacket
{
  uint64_t framenum;
  uint32_t exptime;
  uint32_t packetnum;

  double bunchid;
  uint64_t timestamp;

  uint16_t moduleID;
  uint16_t row;
  uint16_t column;
  uint16_t reserved;

  uint32_t debug;
  uint16_t roundRobin;
  uint8_t detectortype;
  uint8_t headerVersion;
  char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

// Test correctness of structure size
static_assert(sizeof(EGUdpPacket) == BYTES_PER_PACKET);
static_assert(sizeof(EGFrame) == DET_FRAME_STRUCT_BYTES);

namespace eg {

inline std::size_t converted_image_n_bytes(int image_pixel_height,
                                           int image_pixel_width,
                                           int bit_depth)
{
  return image_pixel_width * image_pixel_height * bit_depth / 8;
}

} // namespace eg

#endif // STD_DETECTOR_BUFFER_EIGER_HPP
