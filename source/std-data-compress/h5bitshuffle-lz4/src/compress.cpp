/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <omp.h>
#include <fmt/core.h>
#include <bitshuffle/bitshuffle.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

namespace {
constexpr auto zmq_io_threads = 1;

std::tuple<utils::DetectorConfig, int, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_data_compress_h5bitshuffle_lz4");
  program->add_argument("-t", "--threads")
      .help("number of threads used for compression")
      .action([](const std::string& arg) {
        if (auto value = std::stoi(arg); value < 1 || value > 128 || value % 2 != 0)
          throw std::runtime_error("Unsupported number of cores! Use 1,2,4,8 ...");
        else
          return value;
      })
      .required();
  program->add_argument("-b", "--block_size")
      .help("block size in bytes")
      .action([](const std::string& arg) {
        if (auto value = std::stoi(arg); value < 0)
          throw std::runtime_error("block size must be greater than or equal 0 ...");
        else
          return value;
      })
      .default_value(0);

  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get<int>("--threads"), program->get<int>("--block_size")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, threads, block_size] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_data_compress_h5bitshuffle_lz4", config.log_level};
  auto converted_bytes = utils::converted_image_n_bytes(config);
  omp_set_num_threads(threads);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-image", config.detector_name);
  const auto sink_name = fmt::format("{}-h5bitshuffle-lz4", config.detector_name);

  auto receiver =
      cb::Communicator{{source_name, converted_bytes, utils::slots_number(config)},
                       {source_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender = cb::Communicator{{sink_name, converted_bytes, utils::slots_number(config)},
                                 {sink_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  utils::stats::CompressionStatsCollector stats(config.detector_name,
                                                config.stats_collection_period, converted_bytes);
  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  const auto element_size = config.bit_depth / 8;

  const size_t header_n_bytes = 12;
  auto header_image_n_bytes = htobe64(static_cast<int64_t>(converted_bytes));
  const auto header_block_size = htobe32(static_cast<int32_t>(block_size));

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (meta.status() == std_daq_protocol::good_image) {
        char* compression_buffer = sender.get_data(meta.image_id());

        if (config.detector_type == "pco") {
          converted_bytes = meta.size();
          header_image_n_bytes = htobe64(static_cast<int64_t>(converted_bytes));
        }
        std::memcpy(compression_buffer + 0, &header_image_n_bytes, 8);
        std::memcpy(compression_buffer + 8, &header_block_size, 4);

        int size = bshuf_compress_lz4(receiver.get_data(meta.image_id()), compression_buffer + 12,
                                      converted_bytes / element_size, element_size,
                                      block_size / element_size);

        if (size > 0) {
          meta.set_size(size + header_n_bytes);
          meta.set_compression(std_daq_protocol::h5bitshuffle_lz4);

          std::string meta_buffer_send;
          meta.SerializeToString(&meta_buffer_send);

          sender.send(meta.image_id(), {meta_buffer_send.c_str(), meta_buffer_send.size()},
                      nullptr);
        }
        stats.process(size);
      }
    }
    stats.print_stats();
  }
  return 0;
}
