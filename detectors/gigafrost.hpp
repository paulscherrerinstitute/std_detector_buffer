#ifndef STD_DETECTOR_BUFFER_GIGAFROST_HPP
#define STD_DETECTOR_BUFFER_GIGAFROST_HPP

#include <cstdint>
#include <string>

const std::string DETECTOR_TYPE = "gigafrost";

// 32 bytes header + 7400 max payload
#define BYTES_PER_PACKET 7400 + 32
#define DATA_BYTES_PER_PACKET 7400

#pragma pack(push)
#pragma pack(1)
struct GFUdpPacket
{
  uint8_t protocol_id;                   // Fixed to 0xCB
  uint8_t quadrant_row_length_in_blocks; // Length of quadrant row in 12 pixel blocks -> 24 .. 1008
  uint8_t
      quadrant_rows; // Number of rows in each quadrant. quadrant_rows[0] == SWAP bit. -> 2 .. 1008
  // bit[6-7] == quadrant_id 	GF_NE=3 GF_NW=2 GF_SE=1 GF_SW=0
  // bit[5] == link_id, [0, 1]
  // bit[2-4] == corr_mode,
  // bit[0-1] == unknown?!
  uint8_t status_flags;
  uint32_t scan_id; // Unique ID of the scan.
  // Up to here everything is static per socket and scan.

  uint32_t frame_id;            // Number of the frame in the scan.
  uint16_t image_status_flags;  // bit[15] = do not store, rest is debug bits.
  uint16_t packet_starting_row; // Starting row of the current UDP packet.
  uint64_t image_timing;        // byte[0-4] = timestamp, bytes[3-7] = exposure_time
  uint32_t sync_time;           // No idea
  uint32_t scan_time;           // No Idea

  char data[DATA_BYTES_PER_PACKET];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct GFFrame
{
  // 16 bytes
  uint32_t scan_id;
  uint32_t frame_id;
  uint32_t size_x;
  uint32_t size_y;

  // 24 bytes
  uint32_t scan_time;
  uint32_t sync_time;
  uint64_t frame_timestamp;
  uint64_t exposure_time;

  // 5 bytes
  uint8_t quadrant_id;
  uint8_t link_id;
  uint8_t corr_mode;
  uint8_t rpf; // TODO: No idea what this does.
  uint8_t do_not_store;

  // The struct size needs to be 64 bytes to fit into a cache line.
  int8_t __padding__[64 - 16 - 24 - 5];
};
#pragma pack(pop)

static_assert(sizeof(GFUdpPacket) == BYTES_PER_PACKET);
static_assert(sizeof(GFFrame) == 64u);

#endif // STD_DETECTOR_BUFFER_GIGAFROST_HPP
