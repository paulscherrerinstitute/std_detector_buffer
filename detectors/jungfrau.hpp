#ifndef STD_DETECTOR_BUFFER_JUNGFRAU_HPP
#define STD_DETECTOR_BUFFER_JUNGFRAU_HPP

#include <cstdint>
#include <string>

const std::string DETECTOR_TYPE = "jungfrau";

#define N_MODULES 32
#define BYTES_PER_PACKET 8240
#define DATA_BYTES_PER_PACKET 8192

#define MODULE_X_SIZE 1024
#define MODULE_Y_SIZE 512
#define MODULE_N_PIXELS 524288
#define PIXEL_N_BYTES 2
#define MODULE_N_BYTES 1048576

const size_t N_PACKETS_PER_FRAME = 128;
#define DATA_BYTES_PER_FRAME 1048576

#pragma pack(push)
#pragma pack(1)
struct JFFrame
{
  uint64_t id;
  uint64_t pulse_id;
  uint64_t frame_index;
  uint64_t daq_rec;
  uint64_t n_recv_packets;
  uint64_t module_id;
  uint16_t bit_depth;
  uint16_t pos_y;
  uint16_t pos_x;
};
#pragma pack(pop)

// 48 bytes + 8192 bytes = 8240 bytes
#pragma pack(push)
#pragma pack(2)
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

#endif // STD_DETECTOR_BUFFER_JUNGFRAU_HPP
