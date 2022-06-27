#include <iostream>
#include <zmq.h>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "formats.hpp"
#include "buffer_config.hpp"
#include "buffer_utils.hpp"
#include "core_buffer/sender.hpp"
#include "frame_stat.hpp"

#include "gigafrost.hpp"
#include "packet_udp_receiver.hpp"

using namespace std;
using namespace chrono;
using namespace buffer_config;
using namespace buffer_utils;

// Initialize new frame metadata from first seen packet.
inline void init_frame_metadata(const uint32_t module_size_x,
                                const uint32_t module_size_y,
                                const size_t N_PACKETS_PER_FRAME,
                                const GFUdpPacket& packet,
                                GFFrame& meta)
{
  meta.frame_index = static_cast<uint64_t>(packet.frame_index);
  meta.n_missing_packets = N_PACKETS_PER_FRAME;

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

int main(int argc, char* argv[])
{
  if (argc != 3) {
    cout << endl;
    cout << "Usage: std_udp_recv_gf [detector_json_filename] [module_id]";
    cout << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << "\tmodule_id: id of the module for this process." << endl;
    cout << endl;

    exit(-1);
  }
  const auto config = read_json_config(string(argv[1]));
  const uint16_t module_id = stoi(argv[2]);

  // TODO: Unify naming for bytes and pixels -> module_size tells you nothing.
  // Each line of final image is composed by 2 quadrants side by side.
  const uint32_t module_size_x = config.image_width / 2;
  // Each quadrant is composed by 2 modules streaming interleaved image lines.
  const uint32_t module_size_y = config.image_height / 2 / 2;
  // Each pixel has 12 bytes -> pixel to bytes multiplier is 1.5
  const auto MODULE_N_BYTES = static_cast<size_t>(module_size_x * module_size_y * 1.5);

  const size_t n_rows_per_datagram =
      std::min(static_cast<uint32_t>(DATA_BYTES_PER_PACKET / 1.5 / module_size_x), module_size_y);
  const size_t N_PACKETS_PER_FRAME = std::ceil(module_size_y / n_rows_per_datagram);

  auto ctx = zmq_ctx_new();
  cb::Sender sender{{config.detector_name + std::to_string(module_id),
                     BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
                     DATA_BYTES_PER_PACKET * N_PACKETS_PER_FRAME, RAM_BUFFER_N_SLOTS},
                    ctx};

  PacketUdpReceiver receiver(config.start_udp_port + module_id, sizeof(GFUdpPacket),
                             N_PACKETS_PER_FRAME);
  FrameStats stats(config.detector_name, module_id, STATS_TIME);

  const GFUdpPacket* const packet_buffer =
      reinterpret_cast<GFUdpPacket*>(receiver.get_packet_buffer());
  GFFrame meta = {};
  meta.frame_index = INVALID_FRAME_INDEX;

  char* frame_buffer = new char[MODULE_N_BYTES];

  while (true) {
    // Load n_packets into the packet_buffer.
    const auto n_packets = receiver.receive_many();

    for (int i_packet = 0; i_packet < n_packets; i_packet++) {
      const auto& packet = packet_buffer[i_packet];

      // TODO: Extract the actual packet number.
      const uint64_t packet_num = 0;
      // TODO: This is very probably wrong for GF.
      const size_t frame_buffer_offset = packet_num * DATA_BYTES_PER_PACKET;

      // Packet belongs to the frame we are currently processing.
      if (meta.frame_index == packet.frame_index) {
        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.n_missing_packets -= 1;

        // Copy frame_buffer to ram_buffer and send pulse_id over zmq if last packet in frame.
        if (packet_num == N_PACKETS_PER_FRAME - 1) {
          sender.send(meta.frame_index, reinterpret_cast<char*>(&meta), frame_buffer);
          stats.record_stats(meta.n_missing_packets);
          // Invalidate the current buffer - we already send data out for this one.
          meta.frame_index = INVALID_FRAME_INDEX;
        }
      }
      else {
        // The buffer was not flushed because the last packet from the previous frame was missing.
        if (meta.frame_index != INVALID_FRAME_INDEX) {
          sender.send(meta.frame_index, reinterpret_cast<char*>(&meta), frame_buffer);
          stats.record_stats(meta.n_missing_packets);
        }

        init_frame_metadata(module_size_x, module_size_y, N_PACKETS_PER_FRAME, packet, meta);

        // Accumulate packets data into the frame buffer.
        memcpy(frame_buffer + frame_buffer_offset, packet.data, DATA_BYTES_PER_PACKET);
        meta.n_missing_packets -= 1;
      }
    }
  }

  delete[] frame_buffer;
}
