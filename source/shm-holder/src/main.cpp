/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <thread>

#include <zmq.h>
#include "utils/utils.hpp"

using namespace std::string_literals;

namespace {

utils::DetectorConfig read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("shm_holder");
  program = utils::parse_arguments(std::move(program), argc, argv);
  return utils::read_config_from_json_file(program->get("detector_json_filename"));
}

size_t align_to_2mb(size_t size)
{
  constexpr size_t alignment = 2 * 1024 * 1024;
  return ((size + alignment - 1) / alignment) * alignment;
}

int configure_mmap_flags()
{
  return MAP_SHARED | MAP_LOCKED | MAP_POPULATE;
}

} // namespace

int main(int argc, char* argv[])
{
  const auto config = read_arguments(argc, argv);

  const auto source_name = fmt::format("{}-image", config.detector_name);
  const auto buffer_size =
      align_to_2mb(utils::converted_image_n_bytes(config) * utils::slots_number(config));

  int shm_fd_ = shm_open(source_name.c_str(), O_RDWR | O_CREAT, 0777);
  if (shm_fd_ < 0) throw std::runtime_error(fmt::format("shm_open failed: {}", strerror(errno)));

  if ((ftruncate(shm_fd_, static_cast<off_t>(buffer_size))) == -1)
    throw std::runtime_error(strerror(errno));

  char* buffer_ = static_cast<char*>(
      mmap(nullptr, buffer_size, PROT_WRITE, configure_mmap_flags(), shm_fd_, 0));
  if (buffer_ == MAP_FAILED) throw std::runtime_error(strerror(errno));

  while (true)
    std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
