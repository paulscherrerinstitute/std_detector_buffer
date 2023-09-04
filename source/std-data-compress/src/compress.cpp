/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <omp.h>
#include <fmt/core.h>
#include <bitshuffle/bitshuffle.h>
#include <bitshuffle/bitshuffle.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/args.hpp"
#include "utils/image_size_calc.hpp"
#include "utils/detector_config.hpp"

#include "compression_stats_collector.hpp"


namespace {
constexpr auto zmq_io_threads = 1;

std::tuple<utils::DetectorConfig, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_compress");
  program.add_argument("-t", "--threads")
      .help("number of threads used for compression")
      .action([](const std::string& arg) {
        if (auto value = std::stoi(arg); value < 1 || value > 128 || value % 2 != 0)
          throw std::runtime_error("Unsupported number of cores! Use 1,2,4,8 ...");
        else
          return value;
      })
      .required();

  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get<int>("--threads")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, threads] = read_arguments(argc, argv);
  const size_t image_n_bytes = config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;

  omp_set_num_threads(threads);
  auto converted_bytes = utils::converted_image_n_bytes(config);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-image", config.detector_name);
  const auto sink_name = fmt::format("{}-compressed", config.detector_name);

  auto receiver =
      cb::Communicator{{source_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                       {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender = cb::Communicator{{sink_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sink_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  CompressionStatsCollector stats("std_data_compress", config.detector_name, converted_bytes);
  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  const auto element_size = config.bit_depth / 8;
  const auto block_size = 0;

  const size_t header_n_bytes = 12;
  const auto header_image_n_bytes = __builtin_bswap64(static_cast<int64_t>(image_n_bytes));
  const auto header_block_size = __builtin_bswap32(static_cast<int32_t>(block_size * element_size));

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (meta.status() == std_daq_protocol::good_image) {
        stats.processing_started();

        char* compression_buffer = sender.get_data(meta.image_id());
        
        // How bithusffle + LZ4 HDF5 filter sets the header - needed to decompress HDF5 with filters.
        // https://github.com/kiyo-masui/bitshuffle/blob/04e58bd553304ec26e222654f1d9b6ff64e97d10/src/bshuf_h5filter.c#L166C13-L167C80
        std::memcpy(compression_buffer + 0, &header_image_n_bytes, 8);
        std::memcpy(compression_buffer + 8, &header_block_size, 4);

        int size =
            bshuf_compress_lz4(receiver.get_data(meta.image_id()), compression_buffer + 12,
                               converted_bytes / element_size, element_size, block_size);

        if (size > 0) {
          meta.set_size(size + header_n_bytes);
          meta.set_status(std_daq_protocol::compressed_image);

          std::string meta_buffer_send;
          meta.SerializeToString(&meta_buffer_send);

          sender.send(meta.image_id(), {meta_buffer_send.c_str(), meta_buffer_send.size()},
                      nullptr);
        }
        stats.processing_finished(size);
      }
    }
    stats.print_stats();
  }
  return 0;
}
