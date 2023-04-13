/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <bsread_receiver/receiver.hpp>
#include <fmt/core.h>
#include <zmq.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "utils/args.hpp"
#include "utils/detector_config.hpp"
#include "utils/get_metadata_dtype.hpp"
#include "utils/basic_stats_collector.hpp"
#include "std_daq/image_metadata.pb.h"

namespace {

constexpr std::size_t MAGIC_CAMERA_NUMBER = 4;
constexpr auto type = bsrec::socket_type::pull;
using pulse_id_t = int64_t;

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_bsread_recv");
  program.add_argument("stream_address").help("address to bind input stream");
  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address")};
}
} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address] = read_arguments(argc, argv);
  auto ctx = zmq_ctx_new();

  bsrec::receiver receivers_array[MAGIC_CAMERA_NUMBER] = {
      bsrec::receiver(stream_address, type), bsrec::receiver(stream_address, type),
      bsrec::receiver(stream_address, type), bsrec::receiver(stream_address, type)};

  const std::size_t max_byte_size =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  const auto sync_buffer_name = fmt::format("{}-image", config.detector_name);

  const cb::RamBufferConfig send_buffer_config = {sync_buffer_name, max_byte_size,
                                                  buffer_config::RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig send_comm_config = {sync_buffer_name, ctx, cb::CONN_TYPE_BIND,
                                                   ZMQ_PUB};
  auto sender = cb::Communicator{send_buffer_config, send_comm_config};

  utils::BasicStatsCollector stats("std_bsread_recv", config.detector_name);

  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(utils::get_metadata_dtype(config));
  image_meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);

  std::map<pulse_id_t, std::pair<uint32_t, uint32_t>> pulse_order;

  while (true) {
    stats.processing_started();
    for (auto& receiver : receivers_array) {
      auto msg = receiver.receive();
      if (msg.channels != nullptr) {
        const auto& channels = *msg.channels.get();
        if (!channels.empty()) {
          char* ram_buffer = sender.get_data(msg.pulse_id);
          memcpy(ram_buffer, channels[0].buffer.get(), channels[0].buffer_size);
          pulse_order[msg.pulse_id] = std::make_pair(channels[0].shape[0], channels[0].shape[1]);
        }
      }
    }
    for (auto [pulse, shape] : pulse_order) {
      image_meta.set_image_id(pulse);
      // bsread protocol read shape as [x,y,z,...]
      image_meta.set_width(shape.first);
      image_meta.set_height(shape.second);
      std::string meta_buffer_send;
      image_meta.SerializeToString(&meta_buffer_send);

      sender.send(pulse, meta_buffer_send, nullptr);
    }
    pulse_order.clear();
    stats.processing_finished();
  }
  return 0;
}
