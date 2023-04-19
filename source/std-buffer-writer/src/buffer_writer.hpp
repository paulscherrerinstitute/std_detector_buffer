/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_BUFFER_WRITER_HPP
#define STD_DETECTOR_BUFFER_BUFFER_WRITER_HPP

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <string>
#include <span>

namespace sbw {

class BufferWriter
{
public:
  explicit BufferWriter(std::string root_directory);
  uint64_t write(uint64_t image_id, std::span<char> buffered_data);

private:
  void open_new_file(uint64_t image_id, const std::string& filename);
  uint64_t write_data_and_update_offset(std::span<char> buffered_data);
  [[nodiscard]] std::string get_filename(uint64_t image_id) const;
  [[nodiscard]] std::string get_folder_path(uint64_t image_id) const;

  std::string root_directory_;
  std::string current_filename_;
  std::fstream current_file_;
  uint64_t current_offset_ = 0;
};

} // namespace sbw

#endif // STD_DETECTOR_BUFFER_BUFFER_WRITER_HPP
