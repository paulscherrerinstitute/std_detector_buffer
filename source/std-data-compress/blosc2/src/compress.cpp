/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <blosc2.h>
#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

namespace {

class compression
{
public:
  compression(int threads, std::size_t bit_depth, std::size_t clevel)
  {
    blosc2_cparams compression_params = BLOSC2_CPARAMS_DEFAULTS;
    compression_params.nthreads = static_cast<int16_t>(threads);
    compression_params.typesize = static_cast<int32_t>(bit_depth / 8);
    compression_params.compcode = BLOSC_LZ4;
    compression_params.clevel = clevel;
    compression_params.filters[BLOSC2_MAX_FILTERS - 1] = BLOSC_BITSHUFFLE;
    compression_ctx = blosc2_create_cctx(compression_params);
  }
  ~compression()
  {
    if (compression_ctx != nullptr) blosc2_free_ctx(compression_ctx);
  }
  blosc2_context* ctx() { return compression_ctx; }

private:
  blosc2_context* compression_ctx;
};

constexpr auto zmq_io_threads = 1;

std::tuple<utils::DetectorConfig, int, std::size_t> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_compress_blosc2");
  program.add_argument("-t", "--threads")
      .help("number of threads used for compression")
      .action([](const std::string& arg) {
        if (auto value = std::stoi(arg); value < 1 || value > 128 || value % 2 != 0)
          throw std::runtime_error("Unsupported number of cores! Use 1,2,4,8 ...");
        else
          return value;
      })
      .required();
  program.add_argument("-l", "--level")
      .help("Compression level")
      .action([](const std::string& arg) {
        if (auto value = std::stoi(arg); value < 1 || value > 9)
          throw std::runtime_error(
              fmt::format("Compression level: {} is out of range [1-9]", value));
        else
          return value;
      })
      .default_value(5);

  program = utils::parse_arguments(program, argc, argv);

  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get<int>("--threads"), program.get<int>("--level")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, threads, level] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_data_compress_blosc2", config.log_level};
  auto converted_bytes = utils::converted_image_n_bytes(config);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-image", config.detector_name);
  const auto sink_name = fmt::format("{}-blosc2", config.detector_name);

  auto receiver =
      cb::Communicator{{source_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                       {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender = cb::Communicator{{sink_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sink_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  utils::stats::CompressionStatsCollector stats(config.detector_name, converted_bytes);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  compression compress(threads, config.bit_depth, level);

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (meta.status() == std_daq_protocol::good_image) {
        stats.processing_started();

        auto compressed_size =
            blosc2_compress_ctx(compress.ctx(), receiver.get_data(meta.image_id()), converted_bytes,
                                sender.get_data(meta.image_id()), converted_bytes);

        if (compressed_size > 0) {
          meta.set_size(compressed_size);
          meta.set_compression(std_daq_protocol::blosc2);

          std::string meta_buffer_send;
          meta.SerializeToString(&meta_buffer_send);

          sender.send(meta.image_id(), {meta_buffer_send.c_str(), meta_buffer_send.size()},
                      nullptr);
        }
        stats.processing_finished(compressed_size);
      }
    }
    stats.print_stats();
  }
  return 0;
}
