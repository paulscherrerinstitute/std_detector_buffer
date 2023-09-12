/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <cstring>

#include <zmq.h>
#include <fmt/core.h>

#include "core_buffer/buffer_config.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/jungfrau.hpp"
#include "utils/utils.hpp"

#include "frame_stat.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;

int main(int argc, char* argv[])
{
  auto program = utils::create_parser("std_udp_recv_jf");
  program.add_argument("module_id").scan<'d', uint16_t>();
  program = utils::parse_arguments(program, argc, argv);

  const auto config = utils::read_config_from_json_file(program.get("detector_json_filename"));
  const auto module_id = program.get<uint16_t>("module_id");

  const size_t FRAME_N_BYTES = DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME;

  auto ctx = zmq_ctx_new();
  const auto source_name = fmt::format("{}-{}", config.detector_name, module_id);

  const cb::RamBufferConfig buffer_config = {source_name, FRAME_N_BYTES, RAM_BUFFER_N_SLOTS};
  const cb::CommunicatorConfig comm_config = {source_name, ctx, cb::CONN_TYPE_BIND, ZMQ_PUB};

  cb::Communicator sender{buffer_config, comm_config};

  PacketUdpReceiver receiver(config.start_udp_port + module_id, sizeof(JFUdpPacket),
                             N_PACKETS_PER_FRAME);
  FrameStats stats(config.detector_name, module_id, STATS_TIME);

  const JFUdpPacket* const packet_buffer =
      reinterpret_cast<JFUdpPacket*>(receiver.get_packet_buffer());
  JFFrame meta = {};
  meta.frame_index = INVALID_IMAGE_ID;
  meta.module_id = module_id;
  meta.common.module_id = module_id;

  char* frame_buffer = new char[MODULE_N_BYTES];

  while (true) {
    // Load n_packets into the packet_buffer.
    const auto n_packets = receiver.receive_many();

    for (int i_packet = 0; i_packet < n_packets; i_packet++) {
      const auto& packet = packet_buffer[i_packet];

      const size_t frame_buffer_offset = packet.packetnum * DATA_BYTES_PER_PACKET;

      // Packet belongs to the frame we are currently processing.
      if (meta.frame_index == packet.framenum) {
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;

        // Copy frame_buffer to ram_buffer and send pulse_id over zmq if last packet in frame.
        // TODO: Check comparison between size_t and uint32_t
        if (packet.packetnum == N_PACKETS_PER_FRAME - 1) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)),
                      frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
          // Invalidate the current buffer - we already send data out for this one.
          meta.frame_index = INVALID_IMAGE_ID;
        }
      }
      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.frame_index != INVALID_IMAGE_ID) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)),
                      frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
        }

        // Initialize new frame metadata from first seen packet.
        meta.common.image_id = static_cast<uint64_t>(packet.bunchid);
        meta.common.n_missing_packets = N_PACKETS_PER_FRAME;
        meta.frame_index = packet.framenum;
        meta.daq_rec = packet.debug;

        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;
      }
    }
  }

  delete[] frame_buffer;
}
