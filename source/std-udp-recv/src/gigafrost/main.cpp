/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <cmath>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/formats.hpp"
#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/gigafrost.hpp"
#include "utils/utils.hpp"

#include "frame_stats_collector.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace gf;

// Initialize new frame metadata from first seen packet.
inline void init_frame_metadata(const uint32_t module_size_x,
                                const uint32_t module_size_y,
                                const size_t frame_n_packets,
                                const GFUdpPacket& packet,
                                GFFrame& meta)
{
  meta.common.image_id = static_cast<uint64_t>(packet.frame_index);
  meta.common.n_missing_packets = frame_n_packets;

  meta.scan_id = packet.scan_id;
  meta.size_x = module_size_x;
  meta.size_y = module_size_y;

  meta.scan_time = packet.scan_time;
  meta.sync_time = packet.sync_time;
  // Check struct GFUdpPacket comments for more details.
  meta.frame_timestamp = (packet.image_timing & 0x000000FFFFFFFFFF);
  meta.exposure_time = (packet.image_timing & 0xFFFFFF0000000000) >> 40;

  meta.swapped_rows = packet.quadrant_rows & 0b1;
  meta.quadrant_id = (packet.status_flags & 0b11000000) >> 6;
  meta.link_id = (packet.status_flags & 0b00100000) >> 5;
  meta.corr_mode = (packet.status_flags & 0b00011100) >> 2;

  meta.do_not_store = packet.image_status_flags & 0x8000 >> 15;
}

inline void send_image_id(GFFrame& meta,
                          char* frame_buffer,
                          cb::Communicator& sender,
                          FrameStatsCollector& stats)
{
  sender.send(meta.common.image_id, std::span<char>((char*)(&meta), sizeof(meta)), frame_buffer);
  stats.process(meta.common.n_missing_packets);
  // Invalidate the current buffer - we already send data out for this one.
  meta.common.image_id = INVALID_IMAGE_ID;

  spdlog::debug("sending image_id={}, n_missing_packets={}", meta.common.image_id,
                meta.common.n_missing_packets);
}

inline void process_packet(GFFrame& meta,
                           const GFUdpPacket& packet,
                           char* frame_buffer,
                           const size_t frame_buffer_offset,
                           cb::Communicator& sender,
                           FrameStatsCollector& stats,
                           size_t bytes_per_packet,
                           size_t bytes_of_last_packet,
                           const size_t last_packet_starting_row)
{
  // Record we have received this packet.
  meta.common.n_missing_packets -= 1;

  // If not the last packet just accumulate data in frame_buffer.
  if (packet.packet_starting_row != last_packet_starting_row)
    memcpy(frame_buffer + frame_buffer_offset, packet.data, bytes_per_packet);
  else {
    memcpy(frame_buffer + frame_buffer_offset, packet.data, bytes_of_last_packet);

    // Copy frame_buffer to ram_buffer and send pulse_id over zmq if last packet in frame.
    send_image_id(meta, frame_buffer, sender, stats);
  }
}

int main(int argc, char* argv[])
{
  const std::string prog_name = "std_udp_recv_gf";
  auto program = utils::create_parser(prog_name);
  program->add_argument("module_id").scan<'d', uint16_t>();
  program = utils::parse_arguments(std::move(program), argc, argv);

  const auto detector_config =
      utils::read_config_from_json_file(program->get("detector_json_filename"));
  utils::log::logger l{prog_name, detector_config.log_level};
  const auto module_id = program->get<uint16_t>("module_id");

  const uint32_t width_in_pixels = module_n_x_pixels(detector_config.image_pixel_width);
  const uint32_t height_in_pixels = module_n_y_pixels(detector_config.image_pixel_height);
  const auto bytes_per_module =
      module_n_data_bytes(detector_config.image_pixel_height, detector_config.image_pixel_width);
  const uint32_t rows_per_packet =
      n_rows_per_packet(detector_config.image_pixel_height, detector_config.image_pixel_width);
  auto bytes_per_packet = n_data_bytes_per_packet(detector_config.image_pixel_height,
                                                  detector_config.image_pixel_width);
  const size_t packets_in_frame =
      n_packets_per_frame(detector_config.image_pixel_height, detector_config.image_pixel_width);

  // Calculate if the last packet has the same number of rows as the rest of the packets.
  auto last_packet_n_rows = static_cast<size_t>(height_in_pixels % rows_per_packet);
  // If there is no reminder the last packet has the same number of rows as others.
  if (last_packet_n_rows == 0) last_packet_n_rows = rows_per_packet;

  // Number of data bytes in the last packet.
  auto bytes_of_last_packet = static_cast<size_t>(width_in_pixels * last_packet_n_rows * 1.5);
  if ((last_packet_n_rows % 2 == 1) && (width_in_pixels % 48 != 0)) bytes_of_last_packet += 36;

  // Get offset of last packet in frame to know when to commit frame.
  const size_t start_row_last_packet = height_in_pixels - last_packet_n_rows;
  const size_t bytes_of_frame = (bytes_per_packet * (packets_in_frame - 1)) + bytes_of_last_packet;

  spdlog::debug("bytes_per_module={}, bytes_per_packet={}, packets_in_frame={}, "
                "bytes_of_last_packet={}, bytes_of_frame={}",
                bytes_per_module, bytes_per_packet, packets_in_frame, bytes_of_last_packet,
                bytes_of_frame);

  auto ctx = zmq_ctx_new();
  const auto source_name = fmt::format("{}-{}", detector_config.detector_name, module_id);

  const cb::RamBufferConfig buffer_config = {source_name, bytes_of_frame, RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig comm_config = {source_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB};

  cb::Communicator sender{buffer_config, comm_config};
  PacketUdpReceiver receiver(detector_config.start_udp_port + module_id, sizeof(GFUdpPacket),
                             packets_in_frame);
  FrameStatsCollector stats(detector_config.detector_name, detector_config.stats_collection_period,
                            module_id);

  const GFUdpPacket* const packet_buffer =
      reinterpret_cast<GFUdpPacket*>(receiver.get_packet_buffer());
  GFFrame meta = {};
  meta.common.image_id = INVALID_IMAGE_ID;
  meta.common.module_id = module_id % 8;

  // TODO: Make 64 a const somewhere or read it programmatically (cache line size)
  char* frame_buffer = static_cast<char*>(aligned_alloc(64, bytes_per_module));
  if (frame_buffer == nullptr) throw runtime_error("Cannot allocate frame_buffer.");

  while (true) {
    // Load n_packets into the packet_buffer.
    const auto n_packets = receiver.receive_many();

    for (int i_packet = 0; i_packet < n_packets; i_packet++) {
      const auto& packet = packet_buffer[i_packet];

      // Offset in bytes =  number of rows * row size in pixels * 1.5 (12bit pixels)
      const size_t frame_buffer_offset = packet.packet_starting_row * width_in_pixels * 1.5;

      // Packet belongs to the frame we are currently processing.
      if (meta.common.image_id == packet.frame_index)
        process_packet(meta, packet, frame_buffer, frame_buffer_offset, sender, stats,
                       bytes_per_packet, bytes_of_last_packet, start_row_last_packet);

      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.common.image_id != INVALID_IMAGE_ID)
          send_image_id(meta, frame_buffer, sender, stats);

        init_frame_metadata(width_in_pixels, height_in_pixels, packets_in_frame, packet, meta);

        process_packet(meta, packet, frame_buffer, frame_buffer_offset, sender, stats,
                       bytes_per_packet, bytes_of_last_packet, start_row_last_packet);
      }
    }

    stats.print_stats();
  }

  free(frame_buffer);
}
