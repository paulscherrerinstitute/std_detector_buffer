/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_EIGER_HPP
#define STD_DETECTOR_BUFFER_EIGER_HPP

#include <cstdint>
#include <string>

#include "common.hpp"

namespace eg {

const std::string DETECTOR_TYPE = "eiger";

constexpr inline auto N_MODULES = 1u;
constexpr inline auto BYTES_PER_PACKET = 4144u;
constexpr inline auto DATA_BYTES_PER_PACKET = 4096u;

constexpr inline auto MODULE_X_SIZE = 512u;
constexpr inline auto MODULE_Y_SIZE = 256u;
constexpr inline auto MODULE_N_PIXELS = 131072u;
constexpr inline auto PIXEL_N_BYTES = 2u;
constexpr inline auto GAP_X_MODULE_PIXELS = 2u;
constexpr inline auto GAP_Y_MODULE_PIXELS = 2u;
constexpr inline auto GAP_X_EIGERMOD_PIXELS = 8u;
constexpr inline auto GAP_Y_EIGERMOD_PIXELS = 36u;
constexpr inline auto EXTEND_X_PIXELS = 3u;
constexpr inline auto EXTEND_Y_PIXELS = 1u;

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

  char __padding__[DET_FRAME_STRUCT_BYTES - 18 - 6];
};
#pragma pack(pop)

// for more info on the eiger udp header:
// https://slsdetectorgroup.github.io/devdoc/udpheader.html
#pragma pack(push)
#pragma pack(2)
struct EGUdpPacket
{
  uint64_t frame_num;

  uint32_t exp_length;
  uint32_t packet_number;

  uint64_t detSpec1;

  uint64_t timestamp;

  uint16_t module_id;
  uint16_t row;
  uint16_t column;
  uint16_t detSpec2;

  uint32_t detSpec3;
  uint16_t round_robin;
  uint8_t detector_type;
  uint8_t header_version;
  char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

// Test correctness of structure size
static_assert(sizeof(EGUdpPacket) == BYTES_PER_PACKET);
static_assert(sizeof(EGFrame) == DET_FRAME_STRUCT_BYTES);

} // namespace eg

#endif // STD_DETECTOR_BUFFER_EIGER_HPP
