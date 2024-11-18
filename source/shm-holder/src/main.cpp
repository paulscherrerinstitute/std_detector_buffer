#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <fmt/core.h>
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
  return MAP_SHARED | MAP_LOCKED;
}

// Function for mapping a memory chunk
void mmap_chunk(char* buffer_base,
                int shm_fd,
                size_t offset,
                size_t chunk_size,
                int mmap_flags,
                std::mutex& mmap_mutex)
{
  char* chunk_ptr = static_cast<char*>(
      mmap(buffer_base + offset, chunk_size, PROT_WRITE, mmap_flags, shm_fd, offset));
  if (chunk_ptr == MAP_FAILED) {
    std::lock_guard<std::mutex> lock(mmap_mutex);
    throw std::runtime_error(fmt::format("mmap failed: {}", strerror(errno)));
  }
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

  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();

  // Ensure each chunk is at least 10GB (10 * 1024 * 1024 * 1024 bytes)
  const size_t min_chunk_size = 10L * 1024 * 1024 * 1024; // 10GB
  const size_t num_chunks = (buffer_size + min_chunk_size - 1) / min_chunk_size;

  std::mutex mmap_mutex;
  std::vector<std::thread> threads;

  // Map shared memory in parallel
  for (size_t i = 0; i < num_chunks; ++i) {
    const size_t offset = i * min_chunk_size;
    const size_t actual_chunk_size = std::min(min_chunk_size, buffer_size - offset);

    threads.emplace_back(mmap_chunk, nullptr, shm_fd_, offset, actual_chunk_size,
                         configure_mmap_flags(), std::ref(mmap_mutex));
  }

  // Wait for all threads to complete
  for (auto& thread : threads) {
    thread.join();
  }

  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = end_time - start_time;

  // Print elapsed time
  fmt::print("All mmap tasks completed in {:.2f} seconds\n", elapsed_time.count());

  while (true)
    std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
