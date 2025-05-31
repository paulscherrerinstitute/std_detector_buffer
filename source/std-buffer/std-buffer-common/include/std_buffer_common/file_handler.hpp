/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <blosc2.h>

#include <fstream>
#include <filesystem>
#include <string>
#include <span>
#include <vector>
#include <optional>

namespace sbc {

class FileHandler
{
public:
  explicit FileHandler(std::string root_directory, std::size_t type_size);
  ~FileHandler();

  [[nodiscard]] std::optional<uint64_t> get_first_matching_root_id(uint64_t image_id) const;
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
  [[nodiscard]] std::vector<uint64_t> get_folder_ids_in_root_directory() const;
  [[nodiscard]] std::vector<uint64_t> get_file_ids_in_folder(uint64_t folder_id) const;

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
