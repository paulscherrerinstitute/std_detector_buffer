/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_H5_WRITER
#define STD_DETECTOR_BUFFER_H5_WRITER

#include <memory>
#include <string>
#include <span>
#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/formats.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/detector_config.hpp"

#include <hdf5.h>

class HDF5File
{
public:
  explicit HDF5File(const utils::DetectorConfig& config,
                    const std::string& filename,
                    std::string_view suffix);
  ~HDF5File();

  void write(const std_daq_protocol::ImageMetadata& meta, const char* image);

private:
  static hid_t get_datatype(std::size_t bit_depth);
  void create_file(const std::string& filename);
  void create_datasets(const std::string& detector_name);
  void create_metadata_dataset(hid_t data_group_id);
  void create_image_dataset(hid_t data_group_id);
  void write_image(const char* data, std::size_t data_size);
  void write_meta(const std_daq_protocol::ImageMetadata& meta) const;

  const bool is_h5bitshuffle_lz4_compression;
  const uint32_t image_height;
  const uint32_t image_width;
  const uint32_t image_bit_depth;
  const uint32_t image_size;
  int index;

  hid_t file_id = -1;
  hid_t image_ds = -1;
  hid_t metadata_ds = -1;
};

#endif // STD_DETECTOR_BUFFER_H5_WRITER
