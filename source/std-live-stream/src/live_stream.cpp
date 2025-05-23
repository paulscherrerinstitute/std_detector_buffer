/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <chrono>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

#include "arguments.hpp"
#include "live_stream_stats_collector.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace {
constexpr auto zmq_io_threads = 1;
constexpr auto zmq_sndhwm = 100;

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

void send_array_1_0_stream(void* sender_socket,
                           const std_daq_protocol::ImageMetadata& meta,
                           const char* image_data)
{
  auto data_header = utils::stream::prepare_array10_header(meta);
  auto encoded_c = data_header.c_str();

  zmq_send(sender_socket, encoded_c, data_header.length(), ZMQ_SNDMORE);
  zmq_send(sender_socket, image_data, meta.size(), 0);
}

void send_bsread_stream(void* sender_socket,
                        const std_daq_protocol::ImageMetadata& meta,
                        const char* image_data)
{
  auto [main_header, data_header] = utils::stream::prepare_bsread_headers(meta);
  auto encoded_main_header = main_header.c_str();
  auto encoded_data_header = data_header.c_str();

  zmq_send(sender_socket, encoded_main_header, main_header.length(), ZMQ_SNDMORE);
  zmq_send(sender_socket, encoded_data_header, data_header.length(), ZMQ_SNDMORE);
  zmq_send(sender_socket, image_data, meta.size(), ZMQ_SNDMORE);
  // null message instead of timestamp
  zmq_send(sender_socket, nullptr, 0, 0);
}

std::function<bool(uint64_t)> select_sending_condition(const utils::live_stream_config& conf)
{
  switch (conf.type) {
  case utils::live_stream_config::periodic:
  {
    const auto data_period = 1000ms / (int)conf.value.second;
    auto next_time = steady_clock::now() + data_period;
    return [conf, data_period, next_time, count = 0u](auto) mutable -> bool {
      if (count > 0 && count++ < conf.value.first)
        return true;
      else if (auto now = steady_clock::now(); now > next_time) {
        count = 1;
        next_time = now + data_period;
        return true;
      }
      else
        count = 0;
      return false;
    };
  }
  case utils::live_stream_config::batch:
    return [conf](auto id) mutable -> bool { return (id % conf.value.second) < conf.value.first; };
  default:
    return [](auto) { return true; };
  }
}

} // namespace

int main(int argc, char* argv[])
{
  const auto args = ls::read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_live_stream", args.config.log_level};
  auto should_send_image = select_sending_condition(args.config.ls_configs.at(args.stream_address));

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name_meta =
      fmt::format("{}-{}", args.config.detector_name, args.source_suffix_meta);
  const auto source_name_image =
      fmt::format("{}-{}", args.config.detector_name, args.source_suffix_image);

  auto receiver = cb::Communicator{{source_name_image, utils::converted_image_n_bytes(args.config),
                                    utils::slots_number(args.config)},
                                   {source_name_meta, ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};
  auto sender_socket = bind_sender_socket(ctx, args.stream_address);
  ls::LiveStreamStatsCollector stats(args);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  while (true) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);
      auto image_data = receiver.get_data(meta.image_id());

      if (should_send_image(meta.image_id())) {
        if (args.type == ls::stream_type::array10)
          send_array_1_0_stream(sender_socket, meta, image_data);
        else if (args.type == ls::stream_type::bsread)
          send_bsread_stream(sender_socket, meta, image_data);
      }
      stats.process();
    }
    stats.print_stats();
  }
  return 0;
}
