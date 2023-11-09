/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <fmt/core.h>
#include <zmq.h>
#include <md5.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

namespace {

using pulse_id_t = int64_t;
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;
constexpr char empty_timestamp[16] = {0};

void* bind_sender_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_PUB);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));
  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_bsread_stream");
  program.add_argument("stream_address").help("address to bind output stream");
  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address")};
}

std::string map_bit_depth_to_type(int bit_depth)
{
  if (bit_depth == 8) return "uint8";
  if (bit_depth == 16) return "uint16";
  if (bit_depth == 32) return "uint32";
  return "uint64";
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_bsread_stream", config.log_level};
  const auto compressed_buffer_name = fmt::format("{}-h5bitshuffle-lz4", config.detector_name);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);

  auto receiver = cb::Communicator{{compressed_buffer_name, utils::converted_image_n_bytes(config),
                                    buffer_config::RAM_BUFFER_N_SLOTS},
                                   {compressed_buffer_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_sender_socket(ctx, stream_address);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      auto image_data = receiver.get_data(meta.image_id());

      auto data_header =
          fmt::format(R"({{"htype":"bsr_d-1.1","channels":[{{"name":"{}","shape":[{},{}])"
                      R"(,"type":"{}","compression":"bitshuffle_lz4"}}]}})",
                      meta.pco().bsread_name(), meta.width(), meta.height(),
                      map_bit_depth_to_type(config.bit_depth));

      MD5 digest;
      auto encoded_data_header = data_header.c_str();
      digest.add(encoded_data_header, data_header.length());

      auto main_header = fmt::format(
          R"({{"htype":"bsr_m-1.1","pulse_id":{},"global_timestamp":{{"sec":{},"ns":{}}},"hash":"{}"}})",
          meta.image_id(), meta.pco().global_timestamp_sec(), meta.pco().global_timestamp_ns(),
          digest.getHash());

      auto encoded_main_header = main_header.c_str();
      zmq_send(sender_socket, encoded_main_header, main_header.length(), ZMQ_SNDMORE);
      zmq_send(sender_socket, encoded_data_header, data_header.length(), ZMQ_SNDMORE);
      zmq_send(sender_socket, image_data, meta.size(), ZMQ_SNDMORE);
      zmq_send(sender_socket, empty_timestamp, sizeof(empty_timestamp), 0);
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
