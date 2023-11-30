/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <ranges>
#include <algorithm>

#include <bsread_receiver/receiver.hpp>
#include <fmt/core.h>
#include <zmq.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

namespace {

using pulse_id_t = int64_t;

std::tuple<utils::DetectorConfig, std::string, bsrec::socket_type, std::size_t> read_arguments(
    int argc, char* argv[])
{
  auto program = utils::create_parser("std_bsread_recv");
  program.add_argument("stream_address").help("address to bind input stream");
  program.add_argument("-n", "--number_of_connections")
      .default_value(4)
      .help("Number of parallel connections to the source (4 for PCO cameras).");
  program.add_argument("-t", "--type")
      .default_value(bsrec::socket_type::pull)
      .help("socket type pull or sub")
      .action([](const std::string& value) {
        static const std::unordered_map<std::string, bsrec::socket_type> type_map = {
            {"pull", bsrec::socket_type::pull}, {"sub", bsrec::socket_type::sub}};
        if (auto it = type_map.find(value); it == type_map.end())
          throw std::runtime_error("Invalid choice for --type: " + value);
        else
          return it->second;
      });

  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address"), program.get<bsrec::socket_type>("--type"),
          program.get<std::size_t>("--number_of_connections")};
}

struct tmp_meta
{
  uint32_t width;
  uint32_t height;
  std::string name;
  bsrec::timestamp timestamp;
};

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address, type, number_of_connections] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_bsread_recv", config.log_level};
  auto ctx = zmq_ctx_new();

  std::vector<bsrec::receiver> receivers;
  std::ranges::for_each(std::views::iota(number_of_connections), [&](auto){
    receivers.emplace_back(stream_address, type);
  });
  const std::size_t max_byte_size =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;
  const auto sync_buffer_name = fmt::format("{}-image", config.detector_name);

  const cb::RamBufferConfig send_buffer_config = {sync_buffer_name, max_byte_size,
                                                  buffer_config::RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig send_comm_config = {sync_buffer_name, ctx, cb::CONN_TYPE_BIND,
                                                   ZMQ_PUB};
  auto sender = cb::Communicator{send_buffer_config, send_comm_config};

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period);

  std_daq_protocol::ImageMetadata image_meta;
  image_meta.set_dtype(utils::get_metadata_dtype(config));
  image_meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);
  // TODO: to be improved in future
  std::map<pulse_id_t, tmp_meta> pulse_order;

  while (true) {
    for (auto& receiver : receivers) {
      auto msg = receiver.receive();
      if (msg.channels != nullptr) {
        const auto& channels = *msg.channels.get();
        if (!channels.empty()) {
          char* ram_buffer = sender.get_data(msg.pulse_id);
          memcpy(ram_buffer, channels[0].buffer.get(), channels[0].buffer_size);

          pulse_order.emplace(msg.pulse_id, tmp_meta{channels[0].shape[0], channels[0].shape[1],
                                                     channels[0].name, msg.time});
        }
      }
    }
    for (auto [pulse, m] : pulse_order) {
      image_meta.set_image_id(pulse);
      // bsread protocol read shape as [x,y,z,...]
      image_meta.set_width(m.width);
      image_meta.set_height(m.height);
      image_meta.set_size(m.width * m.height * config.bit_depth / 8);
      image_meta.mutable_pco()->set_bsread_name(m.name);
      image_meta.mutable_pco()->set_global_timestamp_sec(m.timestamp.sec);
      image_meta.mutable_pco()->set_global_timestamp_ns(m.timestamp.nsec);

      std::string meta_buffer_send;
      image_meta.SerializeToString(&meta_buffer_send);

      sender.send(pulse, meta_buffer_send, nullptr);
    }
    pulse_order.clear();
    stats.process();
    stats.print_stats();
  }
  return 0;
}
