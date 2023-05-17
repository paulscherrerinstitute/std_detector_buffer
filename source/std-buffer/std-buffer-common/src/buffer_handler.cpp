/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "buffer_handler.hpp"

namespace sbc {

BufferHandler::BufferHandler(std::string root_directory)
    : root_directory_(std::move(root_directory))
{}

uint64_t BufferHandler::write(uint64_t image_id, std::span<char> buffered_data)
{
  if (std::string filename = get_filename(image_id); filename != current_filename_)
    open_new_file(image_id, filename);
  return write_data_and_update_offset(buffered_data);
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

uint64_t BufferHandler::write_data_and_update_offset(std::span<char> buffered_data)
{
  uint64_t start_position = current_offset_;
  current_file_.write(buffered_data.data(), buffered_data.size());
  current_offset_ += buffered_data.size();
  return start_position;
}

std::string BufferHandler::get_filename(uint64_t image_id) const
{
  return get_folder_path(image_id) + '/' + std::to_string(image_id / 1000) + "000.dat";
}

std::string BufferHandler::get_folder_path(uint64_t image_id) const
{
  return root_directory_ + '/' + std::to_string(image_id / 1000000) + "000000";
}

bool BufferHandler::read(uint64_t image_id, std::span<char> buffered_data, uint64_t offset)
{
  if (std::string filename = get_filename(image_id); filename != current_filename_) {
    current_filename_ = filename;
    current_file_.open(current_filename_, std::ios::in | std::ios::binary);
  }

  if (current_file_.good()) current_file_.seekg(static_cast<long>(offset), std::ios::beg);
  if (current_file_.good()) current_file_.read(buffered_data.data(), buffered_data.size());
  return current_file_.good();
}

} // namespace sbc
