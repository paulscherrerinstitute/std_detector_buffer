/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_handler.hpp"

namespace sbc {

BufferHandler::BufferHandler(std::string root_directory, std::size_t type_size)
    : root_directory_(std::move(root_directory))
    , type_size_(type_size)
{
  blosc2_cparams compression_params = BLOSC2_CPARAMS_DEFAULTS;
  compression_params.typesize = static_cast<int32_t>(type_size);
  compression_params.compcode = BLOSC_BLOSCLZ;
  compression_params.clevel = 5;
  compression_params.nthreads = 4;
  compression_params.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_BITSHUFFLE;
  compression_ctx = blosc2_create_cctx(compression_params);

  blosc2_dparams decompress_params = BLOSC2_DPARAMS_DEFAULTS;
  decompress_params.nthreads = 4;
  decompression_ctx = blosc2_create_dctx(decompress_params);
}

BufferHandler::~BufferHandler()
{
  if (compression_ctx != nullptr) blosc2_free_ctx(compression_ctx);
  if (decompression_ctx != nullptr) blosc2_free_ctx(decompression_ctx);
}

std::pair<uint64_t, uint64_t> BufferHandler::write(uint64_t image_id, std::span<char> buffered_data)
{
  if (std::string filename = get_filename(image_id); filename != current_filename_)
    open_new_file(image_id, filename);
  return write_data_and_update_offset(compress(buffered_data));
}

void BufferHandler::open_new_file(uint64_t image_id, const std::string& filename)
{
  std::filesystem::create_directories(get_folder_path(image_id));
  current_file_.flush();
  current_file_.close();
  current_filename_ = filename;
  current_offset_ = 0;

  current_file_.open(current_filename_, std::ios::out | std::ios::binary | std::ios::app);
}

std::pair<uint64_t, uint64_t> BufferHandler::write_data_and_update_offset(
    std::span<char> buffered_data)
{
  uint64_t start_position = current_offset_;
  current_file_.write(buffered_data.data(), static_cast<std::streamsize>(buffered_data.size()));
  current_offset_ += buffered_data.size();
  return {start_position, buffered_data.size()};
}

std::string BufferHandler::get_filename(uint64_t image_id) const
{
  return get_folder_path(image_id) + '/' + std::to_string(image_id / 1000) + "000.dat";
}

std::string BufferHandler::get_folder_path(uint64_t image_id) const
{
  return root_directory_ + '/' + std::to_string(image_id / 1000000) + "000000";
}

bool BufferHandler::read(uint64_t image_id,
                         std::span<char> buffered_data,
                         uint64_t offset,
                         uint64_t compressed_size)
{
  if (!open_read_file(image_id)) return false;
  buffer.resize(buffered_data.size() + BLOSC2_MAX_OVERHEAD);

  if (current_file_.good()) current_file_.seekg(static_cast<long>(offset), std::ios::beg);
  if (current_file_.good()) current_file_.read(buffer.data(), compressed_size);
  if (current_file_.good())
    return 0 < blosc2_decompress_ctx(decompression_ctx, buffer.data(), compressed_size,
                                     buffered_data.data(), buffered_data.size());
  return false;
}

bool BufferHandler::open_read_file(uint64_t image_id)
{
  if (std::string filename = get_filename(image_id); filename != current_filename_) {
    current_filename_ = filename;
    current_file_.close();
    current_file_.open(current_filename_, std::ios::in | std::ios::binary);
  }
  return current_file_.is_open();
}

std::span<char> BufferHandler::compress(std::span<char> buffered_data)
{
  buffer.resize(buffered_data.size() + BLOSC2_MAX_OVERHEAD);
  if (auto compressed_size =
          blosc2_compress_ctx(compression_ctx, buffered_data.data(), buffered_data.size(),
                              buffer.data(), buffer.size());
      compressed_size > 0)
    return {buffer.data(), (std::size_t)compressed_size};
  else
    return buffered_data;
}

} // namespace sbc
