/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_FORMATS_HPP
#define STD_DETECTOR_BUFFER_FORMATS_HPP

#include <cstdint>



#pragma pack(push)
#pragma pack(1)
struct ModuleFrame
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

enum ImageMetadataDtype {
  uint8=1, uint16=2, uint32=4, uint64=8,
  int8=11, int16=12, int32=14, int64=18,
  float16=22, float32=24, float64=28
};

enum ImageMetadataStatus {
  good_image=0,
  missing_packets=1,
  id_missmatch=2
};

#pragma pack(push)
#pragma pack(1)
struct ImageMetadata
{
  uint64_t id;
  uint64_t height;
  uint64_t width;
  uint16_t dtype;
  uint16_t status;
  uint16_t source_id;
};
#pragma pack(pop)

#endif // STD_DETECTOR_BUFFER_FORMATS_HPP
