/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <cmath>

#include <zmq.h>

#include "core_buffer/formats.hpp"
#include "core_buffer/buffer_config.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "detectors/eiger.hpp"

#include "frame_stat.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace buffer_utils;

// Initialize new frame metadata from first seen packet.
inline void init_frame_metadata(const uint16_t module_id,
                                const uint16_t FRAME_N_PACKETS,
                                const uint16_t bit_depth,
                                const EGUdpPacket& packet,
                                EGFrame& meta)
{
  meta.common.image_id = packet.framenum;
  meta.common.module_id = module_id;
  meta.common.n_missing_packets = FRAME_N_PACKETS;

  meta.bit_depth = bit_depth;
  meta.pos_x = packet.row;
  meta.pos_y = packet.column;

  meta.exptime = packet.exptime;
  meta.bunchid = packet.bunchid;
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
  if (argc != 3) {
    cout << endl;
    cout << "Usage: std_udp_recv_eg [detector_json_filename] [module_id]";
    cout << endl;
    cout << "\tdetector_json_filename: detector detector_config file path." << endl;
    cout << "\tmodule_id: id of the module for this process." << endl;
    cout << endl;

    exit(-1);
  }
  const auto detector_config = read_json_config(string(argv[1]));
  const uint16_t module_id = stoi(argv[2]);

  const size_t FRAME_N_BYTES = MODULE_N_PIXELS * detector_config.bit_depth / 8;
  const size_t N_PACKETS_PER_FRAME = FRAME_N_BYTES / DATA_BYTES_PER_PACKET;

  const cb::RamBufferConfig module_config = {detector_config.detector_name + "-" +
                                                 std::to_string(module_id),
                                             FRAME_N_BYTES, RAM_BUFFER_N_SLOTS};

  auto ctx = zmq_ctx_new();
  cb::Communicator sender{module_config, {ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};
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
      const size_t frame_buffer_offset = packet.packetnum * DATA_BYTES_PER_PACKET;

      // Packet belongs to the frame we are currently processing.
      if (meta.common.image_id == packet.framenum) {
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;

        // Copy frame_buffer to ram_buffer and send pulse_id over zmq if last packet in frame.
        // TODO: Check comparison between size_t and uint32_t
        if (packet.packetnum == N_PACKETS_PER_FRAME - 1) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)), frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
          // Invalidate the current buffer - we already send data out for this one.
          meta.common.image_id = INVALID_IMAGE_ID;
        }
      }
      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.common.image_id != INVALID_IMAGE_ID) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)), frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
        }

        // Initialize new frame metadata from first seen packet.
        init_frame_metadata(module_id, N_PACKETS_PER_FRAME, detector_config.bit_depth, packet, meta);
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;
      }
    }
    stats.process_stats();
  }
  free(frame_buffer);
}
