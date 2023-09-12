/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "read_gains_and_pedestals.hpp"

#include <memory>
#include <source_location>

#include <hdf5.h>
#include <spdlog/spdlog.h>

namespace jf::sdc {
namespace {

parameters decode_parameters(hid_t file_id, const std::string& id, std::size_t image_size)
{
  std::unique_ptr<double> data(new double[N_GAINS * image_size]);

  hid_t dataset_id = H5Dopen2(file_id, id.c_str(), H5P_DEFAULT);
  H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.get());
  H5Dclose(dataset_id);

  parameters params;
  for (auto i = 0u; i < N_GAINS; i++) {
    params[i].reserve(image_size);
    for (auto j = 0u; j < image_size; j++)
      params[i].emplace_back(static_cast<float>(data.get()[i * image_size + j]));
  }
  return params;
}

} // namespace

std::tuple<parameters, parameters> read_gains_and_pedestals(const std::string& filename,
                                                            std::size_t image_size)
{
  spdlog::debug("{}: filename: {}, image_size: {}", std::source_location::current().function_name(),
                filename, image_size);

  hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  auto gains = decode_parameters(file_id, "/gains", image_size);
  auto pedestals = decode_parameters(file_id, "/gainsRMS", image_size);
  H5Fclose(file_id);

  return {gains, pedestals};
}
} // namespace jf::sdc
