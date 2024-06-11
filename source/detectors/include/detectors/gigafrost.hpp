/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_GIGAFROST_HPP
#define STD_DETECTOR_BUFFER_GIGAFROST_HPP

#include <cstdint>
#include <string>
#include <cmath>
#include "common.hpp"

namespace gf {

constexpr inline auto PACKET_N_DATA_BYTES_MAX = 7400u;
// 32 bytes header + 7400 max payload
constexpr inline auto BYTES_PER_PACKET = PACKET_N_DATA_BYTES_MAX + 32u;

#pragma pack(push)
#pragma pack(1)
struct GFUdpPacket
{
  uint8_t protocol_id; // Fixed to 0xCB
  uint8_t
      quadrant_row_length_in_blocks; // Length of quadrant_id row in 12 pixel blocks -> 24 .. 1008
  uint8_t quadrant_rows; // N rows in each quadrant_id. quadrant_rows[0] == SWAP bit. -> 2 .. 1008
  // bit[6-7] == quadrant_id 	GF_NE=3 GF_NW=2 GF_SE=1 GF_SW=0
  // bit[5] == link_id, [0, 1]
  // bit[2-4] == corr_mode,
  // bit[0-1] == quadrant_rows bit 8 and 9
  uint8_t status_flags;
  uint32_t scan_id; // Unique ID of the scan.
  // Up to here everything is static per socket and scan.

  uint32_t frame_index;         // Number of the frame in the scan.
  uint16_t image_status_flags;  // bit[15] = do not store, rest is debug bits.
  uint16_t packet_starting_row; // Starting row of the current UDP packet.
  uint64_t image_timing;        // byte[0-4] = timestamp, bytes[3-7] = exposure_time
  uint32_t sync_time;           // No idea
  uint32_t scan_time;           // No Idea

  char data[PACKET_N_DATA_BYTES_MAX];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct GFFrame
{
  // 18 bytes
  CommonFrame common;

  // 6 bytes
  uint8_t swapped_rows;
  uint8_t link_id;
  uint8_t corr_mode;
  uint8_t quadrant_id;
  uint8_t rpf; // TODO: No idea what this does.
  uint8_t do_not_store;

  // 12 bytes
  uint32_t scan_id;
  uint32_t size_x;
  uint32_t size_y;

  // 24 bytes
  uint32_t scan_time;
  uint32_t sync_time;
  uint64_t frame_timestamp;
  uint64_t exposure_time;

  // The struct size needs to be 64 bytes to fit into a cache line.
  int8_t __padding__[DET_FRAME_STRUCT_BYTES - 18 - 6 - 12 - 24];
};
#pragma pack(pop)

static_assert(sizeof(GFUdpPacket) == BYTES_PER_PACKET);
static_assert(sizeof(GFFrame) == DET_FRAME_STRUCT_BYTES);

enum class quadrant_id : std::size_t
{
  SW = 0,
  SE,
  NW,
  NE
};

// copies are fine
inline constexpr bool operator==(quadrant_id lhs, std::size_t rhs)
{
  return static_cast<std::size_t>(lhs) == rhs;
}

inline constexpr bool operator!=(quadrant_id lhs, std::size_t rhs)
{
  return !(lhs == rhs);
}

inline uint32_t module_n_x_pixels(int image_pixel_width)
{
  // Each line of final image is composed by 2 quadrants side by side.
  return image_pixel_width / 2;
}

inline uint32_t module_n_y_pixels(int image_pixel_height)
{
  // The column is composed by 2 quadrants and each quadrant_id is divided into 2 udp streams that
  // send interleaved rows - each udp stream sends half of the lines from one quadrant_id.
  return image_pixel_height / 2 / 2;
}

inline std::size_t module_n_data_bytes(int image_pixel_height, int image_pixel_width)
{
  return module_n_x_pixels(image_pixel_width) * module_n_y_pixels(image_pixel_height) * 3 / 2;
}

inline uint32_t n_rows_per_packet(int image_pixel_height, int image_pixel_width)
{
  auto module_width = module_n_x_pixels(image_pixel_width);
  auto module_height = module_n_y_pixels(image_pixel_height);

  // Calculate the number of rows in each packet.
  // Do NOT optimize these expressions. The exact form of this calculation is important due to
  // rounding.
  const uint32_t n_12pixel_blocks = module_width / 12;
  const uint32_t n_cache_line_blocks =
      (PACKET_N_DATA_BYTES_MAX / (36 * n_12pixel_blocks)) * n_12pixel_blocks / 2;
  // Each cache line block (64 bytes) has 48 pixels (12 bit pixels)
  return std::min(n_cache_line_blocks * 48 / module_width, module_height);
}

inline std::size_t n_data_bytes_per_packet(int image_pixel_height, int image_pixel_width)
{
  const auto PACKET_N_ROWS = n_rows_per_packet(image_pixel_height, image_pixel_width);
  const auto MODULE_N_X_PIXEL = module_n_x_pixels(image_pixel_width);

  // Calculate the number of data bytes per packet.
  auto PACKET_N_DATA_BYTES = static_cast<size_t>(MODULE_N_X_PIXEL * PACKET_N_ROWS * 1.5);
  if (PACKET_N_ROWS % 2 == 1 && MODULE_N_X_PIXEL % 48 != 0) {
    PACKET_N_DATA_BYTES += 36;
  }
  return PACKET_N_DATA_BYTES;
}

inline std::size_t n_packets_per_frame(int image_pixel_height, int image_pixel_width)
{
  return std::ceil(module_n_y_pixels(image_pixel_height) /
                   (double)n_rows_per_packet(image_pixel_height, image_pixel_width));
}

inline std::size_t last_packet_n_bytes(int image_pixel_height, int image_pixel_width)
{
  auto last_packet_n_rows = module_n_y_pixels(image_pixel_height) %
                            n_rows_per_packet(image_pixel_height, image_pixel_width);
  if (last_packet_n_rows == 0)
    last_packet_n_rows = n_rows_per_packet(image_pixel_height, image_pixel_width);
  return last_packet_n_rows * module_n_x_pixels(image_pixel_width) * 3 / 2;
}

} // namespace gf

#endif // STD_DETECTOR_BUFFER_GIGAFROST_HPP
