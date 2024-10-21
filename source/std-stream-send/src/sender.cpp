/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

#include "sender_stats_collector.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 1000;
constexpr auto zmq_success = 0;
} // namespace

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

std::tuple<utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_stream_send");
  program->add_argument("stream_address").help("address to bind the output stream");
  program->add_argument("image_part")
      .help("0..7 responsible for sending n-th part of image")
      .scan<'d', int>();

  program = utils::parse_arguments(std::move(program), argc, argv);

  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"), program->get<int>("image_part")};
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, image_part] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_stream_send", config.log_level};
  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const auto converted_bytes = utils::converted_image_n_bytes(config);
  const auto start_index = utils::calculate_image_offset(config, image_part);
  if (converted_bytes <= start_index) return 0;
  const auto data_bytes_sent = utils::calculate_image_bytes_sent(config, image_part);

  spdlog::debug("std_stream_send image part = {}, bytes sent = {}, offset = {}", image_part,
                data_bytes_sent, start_index);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto receiver = cb::Communicator{{sync_name, converted_bytes, utils::slots_number(config)},
                                   {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  auto sender_socket = bind_sender_socket(ctx, stream_address);
  gf::send::SenderStatsCollector stats(config.detector_name, config.stats_collection_period,
                                       image_part);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      if (!config.sender_sends_full_images ||
          meta.image_id() % (uint64_t)config.max_number_of_forwarders_spawned ==
              (uint64_t)image_part)
      {
        auto* image_data = receiver.get_data(meta.image_id());

        std::size_t zmq_failed =
            zmq_success == zmq_send(sender_socket, buffer, n_bytes, ZMQ_SNDMORE);
        zmq_failed +=
            zmq_success == zmq_send(sender_socket, image_data + start_index, data_bytes_sent, 0);
        stats.process(zmq_failed);
      }
    }
    stats.print_stats();
  }
  return 0;
}
