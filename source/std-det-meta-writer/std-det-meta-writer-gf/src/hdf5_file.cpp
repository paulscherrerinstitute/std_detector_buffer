/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "hdf5_file.hpp"

#include <spdlog/spdlog.h>
#include <H5version.h>
#include <bitshuffle/bshuf_h5filter.h>

#include "core_buffer/buffer_config.hpp"
#include "utils/image_size_calc.hpp"

namespace {
constexpr size_t max_ik_store = 8192;
} // namespace

typedef struct
{
  uint64_t image_id;
  uint64_t scan_id;
  uint32_t scan_time;
  uint32_t sync_time;
  uint64_t frame_timestamp;
  uint64_t exposure_time;
} h5_metadata;

HDF5File::HDF5File(const utils::DetectorConfig& config, const std::string& filename)
    : gpfs_block_size(config.gpfs_block_size)
    , index(-1)

{
  create_file(filename);
  create_datasets(config.detector_name);
  spdlog::info("Created file filename={}, with file_id={}", filename, file_id);
}

HDF5File::~HDF5File()
{
  spdlog::info("Closing file with file_id={}", file_id);

  if (file_id > 0 && index > 0) {
    const hsize_t metadata_size[1] = {static_cast<hsize_t>(index) + 1};
    H5Dset_extent(metadata_ds, metadata_size);
  }

  if (H5Dclose(metadata_ds) < 0) spdlog::info("Failed closing metadata");
  if (H5Fclose(file_id) < 0) spdlog::info("Failed closing file");
}

void HDF5File::write(const std_daq_protocol::ImageMetadata& meta)
{
  index++;
  spdlog::debug("Writing image_id={} to file_id={} with index={} and size={}", meta.image_id(),
                file_id, index, meta.size());

  auto file_ds = H5Dget_space(metadata_ds);
  if (file_ds < 0) throw std::runtime_error("Cannot get metadata dataset file dataspace.");

  hsize_t current_dims[1];
  if (H5Sget_simple_extent_dims(file_ds, current_dims, nullptr) < 0)
    throw std::runtime_error("Failed to get dataset dimensions.");

  if (const hsize_t required_index = index + 1; required_index > current_dims[0]) {
    const hsize_t records_per_chunk = gpfs_block_size / sizeof(h5_metadata);
    const hsize_t extend_to = ((required_index / records_per_chunk) + 1) * records_per_chunk;

    if (const hsize_t new_size[1] = {extend_to}; H5Dset_extent(metadata_ds, new_size) < 0)
      throw std::runtime_error("Failed to extend dataset.");

    H5Sclose(file_ds);
    file_ds = H5Dget_space(metadata_ds);
  }

  const hsize_t offset[1] = {(hsize_t)index};
  constexpr hsize_t count[1] = {1};
  constexpr hsize_t stride[1] = {1};
  constexpr hsize_t block[1] = {1};

  const auto ram_ds = H5Screate_simple(1, count, nullptr);
  if (ram_ds < 0) throw std::runtime_error("Cannot create metadata ram dataspace.");

  if (H5Sselect_hyperslab(file_ds, H5S_SELECT_SET, offset, stride, count, block) < 0)
    throw std::runtime_error("Cannot select metadata dataset file hyperslab.");

  const hid_t compound_id = create_compound_metadata_structure();

  const h5_metadata data{
      meta.image_id(),       meta.gf().scan_id(),         meta.gf().scan_time(),
      meta.gf().sync_time(), meta.gf().frame_timestamp(), meta.gf().exposure_time()};
  if (H5Dwrite(metadata_ds, compound_id, ram_ds, file_ds, H5P_DEFAULT, &data) < 0)
    throw std::runtime_error("Failed to write metadata.");

  H5Tclose(compound_id);
  H5Sclose(file_ds);
  H5Sclose(ram_ds);
}

void HDF5File::create_file(const std::string& filename)
{
  const auto fapl_id = H5Pcreate(H5P_FILE_ACCESS);
  const auto fcpl_id = H5Pcreate(H5P_FILE_CREATE);

  if (fapl_id == H5I_INVALID_HID) throw std::runtime_error("Error in file access property list.");
  if (H5Pset_alignment(fapl_id, 0, gpfs_block_size) < 0)
    throw std::runtime_error("Cannot set alignment to property list.");

  if (fcpl_id == -1) throw std::runtime_error("Error in file access property list.");
  if (H5Pset_istore_k(fcpl_id, max_ik_store) < 0)
    throw std::runtime_error("Cannot set btree size.");

  file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, fcpl_id, fapl_id);
  if (file_id < 0) throw std::runtime_error("Cannot create output file.");

  H5Pclose(fapl_id);
  H5Pclose(fcpl_id);
}

void HDF5File::create_datasets(const std::string& detector_name)
{
  const auto data_group_id =
      H5Gcreate(file_id, detector_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  if (data_group_id < 0) throw std::runtime_error("Cannot create data group.");

  create_metadata_dataset(data_group_id);
  H5Gclose(data_group_id);
}

void HDF5File::create_metadata_dataset(const hid_t data_group_id)
{
  const hsize_t records_per_chunk = gpfs_block_size / sizeof(h5_metadata);
  constexpr hsize_t initial_dims[1] = {0};
  constexpr hsize_t max_dims[1] = {H5S_UNLIMITED};
  const hsize_t chunk_dims[1] = {records_per_chunk};

  const hid_t dataspace_id = H5Screate_simple(1, initial_dims, max_dims);
  if (dataspace_id < 0) throw std::runtime_error("Cannot create meta dataset space.");

  const hid_t compound_id = create_compound_metadata_structure();
  const hid_t prop_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(prop_id, 1, chunk_dims);

  const size_t cache_size = sizeof(h5_metadata) * records_per_chunk;
  const hid_t access_plist_id = H5Pcreate(H5P_DATASET_ACCESS);
  H5Pset_chunk_cache(access_plist_id, 0, cache_size, 1.0);

  metadata_ds = H5Dcreate(data_group_id, "metadata", compound_id, dataspace_id, H5P_DEFAULT,
                          prop_id, access_plist_id);

  if (metadata_ds < 0) throw std::runtime_error("Cannot create metadata dataset.");

  H5Pclose(access_plist_id);
  H5Tclose(compound_id);
  H5Pclose(prop_id);
  H5Sclose(dataspace_id);
}

hid_t HDF5File::create_compound_metadata_structure()
{
  const hid_t compound_id = H5Tcreate(H5T_COMPOUND, sizeof(h5_metadata));

  H5Tinsert(compound_id, "image_id", HOFFSET(h5_metadata, image_id), H5T_NATIVE_UINT64);
  H5Tinsert(compound_id, "scan_id", HOFFSET(h5_metadata, scan_id), H5T_NATIVE_UINT64);
  H5Tinsert(compound_id, "scan_time", HOFFSET(h5_metadata, scan_time), H5T_NATIVE_UINT32);
  H5Tinsert(compound_id, "sync_time", HOFFSET(h5_metadata, sync_time), H5T_NATIVE_UINT32);
  H5Tinsert(compound_id, "frame_timestamp", HOFFSET(h5_metadata, frame_timestamp),
            H5T_NATIVE_UINT64);
  H5Tinsert(compound_id, "exposure_time", HOFFSET(h5_metadata, exposure_time), H5T_NATIVE_UINT64);

  return compound_id;
}