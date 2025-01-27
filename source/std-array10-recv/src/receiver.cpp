/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <ranges>
#include <algorithm>

#include <fmt/core.h>
#include <zmq.h>
#include <nlohmann/json.hpp>

#include <core_buffer/buffer_utils.hpp>
#include "core_buffer/communicator.hpp"
#include "utils/utils.hpp"
#include "std_buffer/image_metadata.pb.h"

namespace {

std::tuple<utils::DetectorConfig, std::string, int, std::size_t> read_arguments(int argc,
                                                                                char* argv[])
{
  auto program = utils::create_parser("std_array10_recv");
  program->add_argument("stream_address").help("address to bind input stream");
  program->add_argument("-n", "--number_of_connections")
      .default_value(4ul)
      .scan<'u', std::size_t>()
      .help("Number of parallel connections to the source (4 for PCO cameras).");
  program->add_argument("-t", "--type")
      .default_value(ZMQ_PULL)
      .help("socket type pull or sub")
      .action([](const std::string& arg) {
        static const std::unordered_map<std::string, int> type_map = {{"pull", ZMQ_PULL},
                                                                      {"sub", ZMQ_SUB}};
        if (auto it = type_map.find(arg); it == type_map.end())
          throw std::runtime_error("Invalid choice for --type: " + arg);
        else
          return it->second;
      });

  program = utils::parse_arguments(std::move(program), argc, argv);
  return {utils::read_config_from_json_file(program->get("detector_json_filename")),
          program->get("stream_address"), program->get<int>("--type"),
          program->get<std::size_t>("--number_of_connections")};
}

} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address, type, number_of_connections] = read_arguments(argc, argv);
  [[maybe_unused]] utils::log::logger l{"std_array10_recv", config.log_level};
  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, static_cast<int>(number_of_connections));

  std::vector<void*> receiver_sockets;
  std::ranges::for_each(std::views::iota(0u, number_of_connections), [&](auto) {
    receiver_sockets.push_back(buffer_utils::connect_socket(ctx, stream_address, type));
  });

  const std::size_t max_byte_size =
      config.image_pixel_width * config.image_pixel_height * config.bit_depth / 8;

  const auto sync_buffer_name = fmt::format("{}-image", config.detector_name);
  const cb::RamBufferConfig send_buffer_config = {sync_buffer_name, max_byte_size,
                                                  utils::slots_number(config)};
  const cb::CommunicatorConfig send_comm_config = {sync_buffer_name, ctx, cb::CONN_TYPE_BIND,
                                                   ZMQ_PUB};
  auto sender = cb::Communicator{send_buffer_config, send_comm_config};

  utils::stats::TimedStatsCollector stats(config.detector_name, config.stats_collection_period,
                                          stream_address);

  std_daq_protocol::ImageMetadata meta;
  meta.set_dtype(utils::get_metadata_dtype(config));
  meta.set_status(std_daq_protocol::ImageMetadataStatus::good_image);
  meta.set_height(config.image_pixel_height);
  meta.set_width(config.image_pixel_width);
  meta.set_size(max_byte_size);

  auto buffer = new char[max_byte_size + 8];

  std::map<uint64_t, std_daq_protocol::ImageMetadata> image_order;

  while (true) {
    for (auto& socket : receiver_sockets) {
      if (auto n_bytes = zmq_recv(socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
        try {
          auto json = nlohmann::json::parse(std::string_view(buffer, n_bytes));
          meta.set_image_id(json["frame"].get<uint64_t>());
          meta.mutable_gf()->set_exposure_time(json["exposure_time"].get<uint64_t>());
          meta.mutable_gf()->set_frame_timestamp(json["timestamp"].get<uint64_t>());
          meta.mutable_gf()->set_scan_id(json["scan_id"].get<uint32_t>());
          meta.mutable_gf()->set_scan_time(json["scan_time"].get<uint32_t>());
          meta.mutable_gf()->set_sync_time(json["sync_time"].get<uint32_t>());
          meta.mutable_gf()->set_store_image(!json["do_not_store"].get<bool>());
        }
        catch (const nlohmann::json::exception& e) {
        }
        int receive_more;
        size_t sz = sizeof(receive_more);
        if (zmq_getsockopt(socket, ZMQ_RCVMORE, &receive_more, &sz) != -1) {
          if (n_bytes = zmq_recv(socket, buffer, sizeof(buffer), ZMQ_DONTWAIT); n_bytes > 0) {
            std::memcpy(sender.get_data(meta.image_id()), buffer, n_bytes);
            image_order.emplace(meta.image_id(), meta);
          }
        }
        stats.process();
      }
    }
    for (const auto& [pulse, m] : image_order) {
      std::string meta_buffer_send;
      m.SerializeToString(&meta_buffer_send);
      sender.send(pulse, meta_buffer_send, nullptr);
    }
    image_order.clear();
    stats.print_stats();
  }
  return 0;
}
