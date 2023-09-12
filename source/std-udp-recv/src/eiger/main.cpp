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
#include "detectors/eiger.hpp"
#include "utils/utils.hpp"

#include "frame_stat.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;

// Initialize new frame metadata from first seen packet.
inline void init_frame_metadata(const uint16_t module_id,
                                const uint16_t FRAME_N_PACKETS,
                                const uint16_t bit_depth,
                                const EGUdpPacket& packet,
                                EGFrame& meta)
{
  meta.common.image_id = packet.frame_num;
  meta.common.module_id = module_id;
  meta.common.n_missing_packets = FRAME_N_PACKETS;

  meta.bit_depth = bit_depth;
  meta.pos_x = packet.row;
  meta.pos_y = packet.column;
}

inline void send_image_id(EGFrame& meta,
                          char* frame_buffer,
                          cb::Communicator& sender,
                          FrameStats& stats)
{
  sender.send(meta.common.image_id, std::span<char>((char*)(&meta), sizeof(meta)), frame_buffer);
  stats.record_stats(meta.common.n_missing_packets);
  // Invalidate the current buffer - we already send data out for this one.
  meta.common.image_id = INVALID_IMAGE_ID;
}

int main(int argc, char* argv[])
{
  auto program = utils::create_parser("std_udp_recv_eg");
  program.add_argument("module_id").scan<'d', uint16_t>();
  program = utils::parse_arguments(program, argc, argv);

  const auto detector_config =
      utils::read_config_from_json_file(program.get("detector_json_filename"));
  const auto module_id = program.get<uint16_t>("module_id");

  const size_t FRAME_N_BYTES = MODULE_N_PIXELS * detector_config.bit_depth / 8;
  const size_t N_PACKETS_PER_FRAME = FRAME_N_BYTES / DATA_BYTES_PER_PACKET;

  auto ctx = zmq_ctx_new();
  const auto source_name = fmt::format("{}-{}", detector_config.detector_name, module_id);

  const cb::RamBufferConfig buffer_config = {source_name, FRAME_N_BYTES, RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig comm_config = {source_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB};

  cb::Communicator sender{buffer_config, comm_config};
  PacketUdpReceiver receiver(detector_config.start_udp_port + module_id, sizeof(EGUdpPacket),
                             N_PACKETS_PER_FRAME);
  FrameStats stats(detector_config.detector_name, module_id, STATS_TIME);

  const EGUdpPacket* const packet_buffer =
      reinterpret_cast<EGUdpPacket*>(receiver.get_packet_buffer());
  EGFrame meta = {};
  meta.common.image_id = INVALID_IMAGE_ID;
  meta.common.module_id = module_id;

  // TODO: Make 64 a const somewhere or read it programmatically (cache line size)
  char* frame_buffer = new char[FRAME_N_BYTES];

  while (true) {
    // Load n_packets into the packet_buffer.
    const auto n_packets = receiver.receive_many();

    for (int i_packet = 0; i_packet < n_packets; i_packet++) {
      const auto& packet = packet_buffer[i_packet];
      const size_t frame_buffer_offset = packet.packet_number * DATA_BYTES_PER_PACKET;

      // Packet belongs to the frame we are currently processing.
      if (meta.common.image_id == packet.frame_num) {
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;

        // Copy frame_buffer to ram_buffer and send pulse_id over zmq if last packet in frame.
        // TODO: Check comparison between size_t and uint32_t
        if (packet.packet_number == N_PACKETS_PER_FRAME - 1) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)),
                      frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
          // Invalidate the current buffer - we already send data out for this one.
          meta.common.image_id = INVALID_IMAGE_ID;
        }
      }
      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.common.image_id != INVALID_IMAGE_ID) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)),
                      frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
        }

        // Initialize new frame metadata from first seen packet.
        init_frame_metadata(module_id, N_PACKETS_PER_FRAME, detector_config.bit_depth, packet,
                            meta);
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;
      }
    }
    stats.process_stats();
  }
  free(frame_buffer);
}
