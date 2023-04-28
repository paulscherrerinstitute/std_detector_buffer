/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_writer.hpp"

namespace sbw {

BufferWriter::BufferWriter(std::string root_directory)
    : root_directory_(std::move(root_directory))
{}

uint64_t BufferWriter::write(uint64_t image_id, std::span<char> buffered_data)
{
  if (std::string filename = get_filename(image_id); filename != current_filename_)
    open_new_file(image_id, filename);
  return write_data_and_update_offset(buffered_data);
}

void BufferWriter::open_new_file(uint64_t image_id, const std::string& filename)
{
  std::filesystem::create_directories(get_folder_path(image_id));
  current_file_.flush();
  current_file_.close();
  current_filename_ = filename;
  current_offset_ = 0;

  current_file_.open(current_filename_, std::ios::out | std::ios::binary | std::ios::app);
}

uint64_t BufferWriter::write_data_and_update_offset(std::span<char> buffered_data)
{
  uint64_t start_position = current_offset_;
  current_file_.write(buffered_data.data(), buffered_data.size());
  current_offset_ += buffered_data.size();
  return start_position;
}

std::string BufferWriter::get_filename(uint64_t image_id) const
{
  return get_folder_path(image_id) + '/' + std::to_string(image_id / 1000) + "000.dat";
}

std::string BufferWriter::get_folder_path(uint64_t image_id) const
{
  return root_directory_ + '/' + std::to_string(image_id / 1000000) + "000000";
}

} // namespace sbw
