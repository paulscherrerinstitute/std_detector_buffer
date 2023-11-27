/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_H5_WRITER
#define STD_DETECTOR_BUFFER_H5_WRITER

#include <memory>
#include <string>
#include <span>
#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/formats.hpp"
#include "std_buffer/image_metadata.pb.h"

#include <hdf5.h>

class H5Writer
{

  const std::string detector_name_;
  const bool is_h5bitshuffle_lz4_compression;

  static const int64_t NO_RUN_ID = -1;

  // Run specific variables.
  uint64_t current_run_id_ = -1L;
  uint32_t image_y_size_ = 0;
  uint32_t image_x_size_ = 0;
  uint32_t bit_depth_ = 0;
  uint32_t image_n_bytes_ = 0;

  // Open file specific variables.
  hid_t file_id_ = -1;
  hid_t image_data_dataset_ = -1;
  hid_t image_id_dataset_ = -1;
  hid_t status_dataset_ = -1;

  static hid_t get_datatype(std::size_t bit_depth);
  void open_file(const std::string& output_file, uint32_t n_images);
  void close_file(uint32_t highest_written_index);
  void create_metadata_datasets(uint32_t n_images, hid_t data_group_id);
  void create_image_dataset(uint32_t n_images, hid_t data_group_id);

public:
  explicit H5Writer(std::string detector_name, std::string_view suffix);
  ~H5Writer();

  void open_run(const std::string& output_file,
                uint64_t run_id,
                int n_images,
                int image_y_size,
                int image_x_size,
                int bit_depth);
  void close_run(uint32_t highest_written_index);

  void write_data(uint64_t run_id, uint32_t index, size_t data_size, const char* data) const;
  void write_meta(uint64_t run_id, uint32_t index, const std_daq_protocol::ImageMetadata& meta) const;
};

#endif // STD_DETECTOR_BUFFER_H5_WRITER
