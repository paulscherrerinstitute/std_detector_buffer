/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include "std_buffer/image_metadata.pb.h"
#include "utils/detector_config.hpp"

#include <hdf5.h>

class HDF5File
{
public:
  explicit HDF5File(const utils::DetectorConfig& config, const std::string& filename);
  ~HDF5File();

  void write(const std_daq_protocol::ImageMetadata& meta);

private:
  void create_file(const std::string& filename);
  void create_datasets(const std::string& detector_name);
  void create_metadata_dataset(hid_t data_group_id);
  static hid_t create_compound_metadata_structure();

  const size_t gpfs_block_size;
  int index;

  hid_t file_id = -1;
  hid_t metadata_ds = -1;
};
