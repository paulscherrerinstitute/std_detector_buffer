/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <zmq.h>
#include <cstring>

#include "core_buffer/formats.hpp"
#include "core_buffer/buffer_config.hpp"
#include "core_buffer/buffer_utils.hpp"
#include "core_buffer/communicator.hpp"
#include "frame_stat.hpp"

#include "jungfrau.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace buffer_utils;

int main(int argc, char* argv[])
{
  if (argc != 3) {
    cout << endl;
    cout << "Usage: std_udp_recv_jf [detector_json_filename] [module_id]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << "\tmodule_id: id of the module for this process." << endl;
    cout << endl;

    exit(-1);
  }
  const auto config = read_json_config(string(argv[1]));
  const uint16_t module_id = stoi(argv[2]);

  auto ctx = zmq_ctx_new();
  cb::Communicator sender{
      {config.detector_name + "-" + std::to_string(module_id),
       DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME, RAM_BUFFER_N_SLOTS},
      {ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}};

  PacketUdpReceiver receiver(config.start_udp_port + module_id, sizeof(JFUdpPacket),
                             N_PACKETS_PER_FRAME);
  FrameStats stats(config.detector_name, module_id, STATS_TIME);

  const JFUdpPacket* const packet_buffer =
      reinterpret_cast<JFUdpPacket*>(receiver.get_packet_buffer());
  JFFrame meta = {};
  meta.frame_index = INVALID_IMAGE_ID;
  meta.module_id = module_id;

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
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)), frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
          // Invalidate the current buffer - we already send data out for this one.
          meta.frame_index = INVALID_IMAGE_ID;
        }
      }
      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.frame_index != INVALID_IMAGE_ID) {
          sender.send(meta.common.image_id, std::span<char>((char*)&meta, sizeof(meta)), frame_buffer);
          stats.record_stats(meta.common.n_missing_packets);
        }

        // Initialize new frame metadata from first seen packet.
        meta.common.image_id = static_cast<uint64_t>(packet.bunchid);
        meta.common.n_missing_packets = N_PACKETS_PER_FRAME;
        meta.frame_index = packet.framenum;
        meta.daq_rec = packet.debug;
        meta.module_id = module_id;

        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.common.n_missing_packets -= 1;
      }
    }
  }

  delete[] frame_buffer;
}
