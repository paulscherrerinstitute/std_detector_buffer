/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_JUNGFRAU_HPP
#define STD_DETECTOR_BUFFER_JUNGFRAU_HPP

#include <cstdint>
#include <string_view>
#include "common.hpp"

namespace jf {

constexpr inline auto BYTES_PER_PACKET = 8240u;
constexpr inline auto DATA_BYTES_PER_PACKET = 8192u;

constexpr inline auto MODULE_X_SIZE = 1024u;
constexpr inline auto MODULE_Y_SIZE = 512u;
constexpr inline auto MODULE_N_PIXELS = MODULE_Y_SIZE * MODULE_X_SIZE;
constexpr inline auto PIXEL_N_BYTES = 2u;
constexpr inline auto MODULE_N_BYTES = PIXEL_N_BYTES * MODULE_N_PIXELS;

constexpr inline auto N_PACKETS_PER_FRAME = 128u;

// 6*8 = 48 bytes of data + 12 bytes of padding == 64 bytes (cache line)
#pragma pack(push, 1)
struct JFFrame
{
  // 18 bytes.
  CommonFrame common;

  // 32 bytes.
  uint64_t id;
  uint64_t frame_index;
  uint64_t daq_rec;
  uint64_t module_id;

  char __padding__[DET_FRAME_STRUCT_BYTES - 18 - 32];
};
#pragma pack(pop)

// 48 bytes + 8192 bytes = 8240 bytes
#pragma pack(push, 2)
struct JFUdpPacket
{
  uint64_t framenum;
  uint32_t exptime;
  uint32_t packetnum;

  double bunchid;
  uint64_t timestamp;

  uint16_t moduleID;
  uint16_t row;
  uint16_t column;
  uint16_t zCoord;

  uint32_t debug;
  uint16_t roundRobin;
  uint8_t detectortype;
  uint8_t headerVersion;
  char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

// Test correctness of structure size
static_assert(sizeof(JFUdpPacket) == BYTES_PER_PACKET);
static_assert(sizeof(JFFrame) == DET_FRAME_STRUCT_BYTES);

} // namespace jf

#endif // STD_DETECTOR_BUFFER_JUNGFRAU_HPP
