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

  static hid_t get_datatype(int bit_depth);
  void open_file(const std::string& output_file, uint32_t n_images);
  void close_file(const uint32_t highest_written_index);

public:
  H5Writer(std::string detector_name);
  ~H5Writer();

  void open_run(const std::string& output_file,
                const uint64_t run_id,
                const int n_images,
                const int image_y_size,
                const int image_x_size,
                const int bit_depth);
  void close_run(const uint32_t highest_written_index);

  void write_data(const uint64_t run_id,
                  const uint32_t index,
                  const size_t data_size,
                  const char* data);
  void write_meta(const uint64_t run_id,
                  const uint32_t index,
                  const std_daq_protocol::ImageMetadata& meta);
};

#endif // STD_DETECTOR_BUFFER_H5_WRITER
