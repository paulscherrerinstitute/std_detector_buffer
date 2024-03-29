/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_BUFFER_HANDLER_HPP
#define STD_DETECTOR_BUFFER_BUFFER_HANDLER_HPP

#include <blosc2.h>

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <string>
#include <span>
#include <vector>

namespace sbc {

class BufferHandler
{
public:
  explicit BufferHandler(std::string root_directory, std::size_t type_size);
  ~BufferHandler();
  std::pair<uint64_t, uint64_t> write(uint64_t image_id, std::span<char> buffered_data);
  bool read(uint64_t image_id,
            std::span<char> buffered_data,
            uint64_t offset,
            uint64_t compressed_size);

private:
  void open_new_file(uint64_t image_id, const std::string& filename);
  std::pair<uint64_t, uint64_t> write_data_and_update_offset(std::span<char> buffered_data);
  std::span<char> compress(std::span<char> buffered_data);
  bool open_read_file(uint64_t image_id);
  [[nodiscard]] std::string get_filename(uint64_t image_id) const;
  [[nodiscard]] std::string get_folder_path(uint64_t image_id) const;

  std::string root_directory_;
  std::string current_filename_;
  std::fstream current_file_;
  uint64_t current_offset_ = 0;
  std::size_t type_size_;
  blosc2_context* compression_ctx;
  blosc2_context* decompression_ctx;
  std::vector<char> buffer;
};

} // namespace sbc

#endif // STD_DETECTOR_BUFFER_BUFFER_HANDLER_HPP
