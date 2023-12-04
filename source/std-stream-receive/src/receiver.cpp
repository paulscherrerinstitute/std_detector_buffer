/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/communicator.hpp"
#include "core_buffer/ram_buffer.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "std_buffer/image_metadata.pb.h"
#include "utils/utils.hpp"

#include "receiver_stats_collector.hpp"

namespace {
constexpr auto zmq_io_threads = 1;
} // namespace

void* zmq_socket_connect(void* ctx, const std::string& stream_address)
{
  void* socket = buffer_utils::create_socket(ctx, ZMQ_SUB);
  if (zmq_connect(socket, stream_address.c_str())) throw std::runtime_error(zmq_strerror(errno));
  return socket;
}

std::tuple<utils::DetectorConfig, std::string, int> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_stream_receive_gf");
  program->add_argument("stream_address").help("address to bind input stream");
  program->add_argument("image_part")
      .help("0..7 responsible for sending n-th part of image")
      .scan<'d', int>();
  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"), program->get<int>("image_part")};
}

bool received_successfully_data(void* socket, char* buffer, std::size_t size)
{
  int receive_more;
  size_t sz = sizeof(receive_more);
  return zmq_getsockopt(socket, ZMQ_RCVMORE, &receive_more, &sz) != -1 &&
         zmq_recv(socket, buffer, size, ZMQ_DONTWAIT) > 0;
}

int main(int argc, char* argv[])
{
  const auto [config, stream_address, image_part] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_stream_receive", config.log_level};
  const auto image_name = fmt::format("{}-image", config.detector_name);
  const auto sync_name = fmt::format("{}-sync", config.detector_name);
  const auto converted_bytes = utils::converted_image_n_bytes(config);
  const auto start_index = image_part * utils::max_single_sender_size(config);
  if (converted_bytes <= start_index) return 0;
  const auto data_bytes_sent =
      std::min(converted_bytes - start_index, utils::max_single_sender_size(config));

  gf::rec::ReceiverStatsCollector stats(config.detector_name, config.stats_collection_period,
                                        image_part);

  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, zmq_io_threads);

  auto sender = cb::Communicator{{image_name, converted_bytes, buffer_config::RAM_BUFFER_N_SLOTS},
                                 {sync_name, ctx, cb::CONN_TYPE_CONNECT, ZMQ_PUSH}};

  auto socket = zmq_socket_connect(ctx, stream_address);
  while (true) {
    unsigned int zmq_fails = 0;
    meta.set_image_id(0);
    if (auto n_bytes = zmq_recv(socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
        meta.ParseFromArray(buffer, n_bytes);
      char* data = sender.get_data(meta.image_id());
      if (received_successfully_data(socket, data + start_index, data_bytes_sent))
        sender.send(meta.image_id(), std::span{buffer, static_cast<std::size_t>(n_bytes)}, nullptr);
      else
        zmq_fails++;
      stats.process(zmq_fails, meta.image_id());
    }
    stats.print_stats();
  }
  return 0;
}
