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

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (meta.status() == std_daq_protocol::good_image) {
        stats.processing_started();

        auto element_size = config.bit_depth / 8;
        int size =
            bshuf_compress_lz4(receiver.get_data(meta.image_id()), sender.get_data(meta.image_id()),
                               converted_bytes / element_size, element_size, 0);

        if (size > 0) {
          meta.set_size(size);
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
