/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "hdf5_file.hpp"

#include <spdlog/spdlog.h>
#include <H5version.h>
#include <bitshuffle/bshuf_h5filter.h>

#include "core_buffer/buffer_config.hpp"

namespace {

constexpr size_t gpfs_block_size = 16777216;
constexpr size_t max_ik_store = 8192;

} // namespace

typedef struct
{
  uint64_t image_id;
  uint64_t status;
} h5_metadata;

HDF5File::HDF5File(const utils::DetectorConfig& config,
                   const std::string& filename,
                   std::string_view suffix)
    : is_h5bitshuffle_lz4_compression(suffix == "h5bitshuffle-lz4")
    , image_height(config.image_pixel_height)
    , image_width(config.image_pixel_width)
    , image_bit_depth(config.bit_depth)
    , index(0)

{
  create_file(filename);
  create_datasets(config.detector_name);
  spdlog::info("Created file filename={}, with file_id={}", filename, file_id);
}

HDF5File::~HDF5File()
{
  spdlog::info("Closing file with file_id={}", file_id);
  H5Dclose(metadata_ds);
  H5Dclose(image_ds);
  H5Fclose(file_id);
}

void HDF5File::write(std::span<char> image, const std_daq_protocol::ImageMetadata& meta)
{
  spdlog::debug("Writing image_id={} to file_id={} with index={}", meta.image_id(), file_id, index);
  write_meta(meta);
  write_image(image);
  index++;
}

hid_t HDF5File::get_datatype(std::size_t bit_depth)
{
  static std::map<std::size_t, hid_t> bit_depth_map = {
      {8, H5T_NATIVE_UINT8}, {16, H5T_NATIVE_UINT16}, {32, H5T_NATIVE_UINT32}};

  if (const auto it = bit_depth_map.find(bit_depth); it != bit_depth_map.end()) return it->second;
  throw std::runtime_error(fmt::format("Unsupported bits per pixel: {}", bit_depth));
}

void HDF5File::create_file(const std::string& filename)
{
  auto fapl_id = H5Pcreate(H5P_FILE_ACCESS);
  auto fcpl_id = H5Pcreate(H5P_FILE_CREATE);
  try {
    if (fapl_id == H5I_INVALID_HID) throw std::runtime_error("Error in file access property list.");
    if (H5Pset_alignment(fapl_id, 0, gpfs_block_size) < 0)
      throw std::runtime_error("Cannot set alignment to property list.");

    if (fcpl_id == -1) throw std::runtime_error("Error in file access property list.");
    if (H5Pset_istore_k(fcpl_id, max_ik_store) < 0)
      throw std::runtime_error("Cannot set btree size.");

    file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, fcpl_id, fapl_id);
    if (file_id < 0) throw std::runtime_error("Cannot create output file.");
    H5Pclose(fcpl_id);
  }
  catch (const std::exception& e) {
    H5Pclose(fapl_id);
    H5Pclose(fcpl_id);
    throw;
  }
}

void HDF5File::create_datasets(const std::string& detector_name)
{
  auto data_group_id =
      H5Gcreate(file_id, detector_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  try {
    if (data_group_id < 0) throw std::runtime_error("Cannot create data group.");

    create_metadata_dataset(data_group_id);
    create_image_dataset(data_group_id);
  }
  catch (const std::exception& e) {
    H5Gclose(data_group_id);
    throw;
  }
}

void HDF5File::create_image_dataset(hid_t data_group_id)
{
  auto dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
  if (dcpl_id < 0) throw std::runtime_error("Error in creating dataset create property list.");

  hsize_t image_dataset_chunking[] = {1, image_height, image_width};
  if (H5Pset_chunk(dcpl_id, 3, image_dataset_chunking) < 0)
    throw std::runtime_error("Cannot set image dataset chunking.");

  hsize_t dims[3] = {0, image_height, image_width};
  hsize_t max_dims[3] = {H5S_UNLIMITED, image_height, image_width};
  auto image_space_id = H5Screate_simple(3, dims, max_dims);
  if (image_space_id < 0) throw std::runtime_error("Cannot create image dataset space.");

  if (is_h5bitshuffle_lz4_compression) {
    bshuf_register_h5filter();
    uint filter_prop[] = {0, BSHUF_H5_COMPRESS_LZ4};
    if (H5Pset_filter(dcpl_id, BSHUF_H5FILTER, H5Z_FLAG_MANDATORY, 2, filter_prop) < 0)
      throw std::runtime_error("Cannot set compression filter on dataset.");
  }

  image_ds = H5Dcreate(data_group_id, "data", get_datatype(image_bit_depth), image_space_id,
                       H5P_DEFAULT, dcpl_id, H5P_DEFAULT);
  if (image_ds < 0) throw std::runtime_error("Cannot create image dataset.");

  H5Sclose(image_space_id);
  H5Pclose(dcpl_id);
}

void HDF5File::create_metadata_dataset(hid_t data_group_id)
{
  hsize_t records_per_chunk = gpfs_block_size / sizeof(h5_metadata);
  hsize_t initial_dims[1] = {0};
  hsize_t max_dims[1] = {H5S_UNLIMITED};
  hsize_t chunk_dims[1] = {records_per_chunk};

  hid_t dataspace_id = H5Screate_simple(1, initial_dims, max_dims);
  try {
    if (dataspace_id < 0) throw std::runtime_error("Cannot create meta dataset space.");

    hid_t compound_id = H5Tcreate(H5T_COMPOUND, sizeof(h5_metadata));
    H5Tinsert(compound_id, "image_id", HOFFSET(h5_metadata, image_id), H5T_NATIVE_UINT64);
    H5Tinsert(compound_id, "status", HOFFSET(h5_metadata, status), H5T_NATIVE_UINT64);

    hid_t prop_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(prop_id, 1, chunk_dims);

    metadata_ds = H5Dcreate(data_group_id, "metadata", compound_id, dataspace_id, H5P_DEFAULT,
                            prop_id, H5P_DEFAULT);

    H5Tclose(compound_id);
    H5Pclose(prop_id);
    if (metadata_ds < 0) throw std::runtime_error("Cannot create metadata dataset.");
  }
  catch (const std::exception& e) {
    H5Sclose(dataspace_id);
    throw;
  }
}

void HDF5File::write_image(std::span<char> data) const
{
  hsize_t offset[3] = {index, 0, 0};

  if (H5Dwrite_chunk(image_ds, H5P_DEFAULT, 0, offset, data.size_bytes(), data.data()) < 0)
    throw std::runtime_error("Cannot write data to image dataset.");
}

void HDF5File::write_meta(const std_daq_protocol::ImageMetadata& meta) const
{
  auto file_ds = H5Dget_space(metadata_ds);
  if (file_ds < 0) throw std::runtime_error("Cannot get metadata dataset file dataspace.");

  hsize_t offset[1] = {index};
  hsize_t count[1] = {1};
  hsize_t stride[1] = {1};
  hsize_t block[1] = {1};

  auto ram_ds = H5Screate_simple(1, count, nullptr);
  if (ram_ds < 0) throw std::runtime_error("Cannot create metadata ram dataspace.");

  if (H5Sselect_hyperslab(file_ds, H5S_SELECT_SET, offset, stride, count, block) < 0)
    throw std::runtime_error("Cannot select metadata dataset file hyperslab.");

  hid_t compound_type = H5Tcreate(H5T_COMPOUND, sizeof(h5_metadata));
  H5Tinsert(compound_type, "image_id", HOFFSET(h5_metadata, image_id), H5T_NATIVE_UINT64);
  H5Tinsert(compound_type, "status", HOFFSET(h5_metadata, status), H5T_NATIVE_UINT64);

  h5_metadata data{meta.image_id(), (uint64_t)meta.status()};
  if (H5Dwrite(metadata_ds, compound_type, ram_ds, file_ds, H5P_DEFAULT, &data) < 0)
    throw std::runtime_error("Failed to write metadata.");

  H5Tclose(compound_type);
  H5Sclose(file_ds);
  H5Sclose(ram_ds);
}
