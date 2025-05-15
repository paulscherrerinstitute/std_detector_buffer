/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

constexpr inline auto DET_FRAME_STRUCT_BYTES = 64u;
constexpr inline auto INVALID_IMAGE_ID = (uint64_t)-1;

#pragma pack(push)
#pragma pack(1)
struct CommonFrame
{
  // 18 bytes
  uint64_t image_id;
  uint64_t n_missing_packets;
  uint16_t module_id;
};
#pragma pack(pop)
