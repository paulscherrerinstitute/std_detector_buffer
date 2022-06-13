#include <cstring>
#include <jungfrau.hpp>
#include "FrameUdpReceiver.hpp"

using namespace std;

template<typename T, size_t N_RECV_MSG>
FrameUdpReceiver<T,N_RECV_MSG>::FrameUdpReceiver(const uint16_t port, const int module_id) :
    module_id_(module_id)
{
  udp_receiver_.bind(port);

  for (size_t i = 0; i < N_RECV_MSG; i++) {
    recv_buff_ptr_[i].iov_base = (void*) &(packet_buffer_[i]);
    recv_buff_ptr_[i].iov_len = sizeof(jungfrau_packet);

    msgs_[i].msg_hdr.msg_iov = &recv_buff_ptr_[i];
    msgs_[i].msg_hdr.msg_iovlen = 1;
    msgs_[i].msg_hdr.msg_name = &sock_from_[i];
    msgs_[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
  }
}

template<typename T, size_t N_RECV_MSG>
FrameUdpReceiver<T,N_RECV_MSG>::~FrameUdpReceiver() {
  udp_receiver_.disconnect();
}

template<typename T, size_t N_RECV_MSG>
inline void FrameUdpReceiver<T,N_RECV_MSG>::init_frame(
    ModuleFrame& frame_metadata, const int i_packet)
{
  frame_metadata.pulse_id = static_cast<uint64_t>(packet_buffer_[i_packet].bunchid);
  frame_metadata.frame_index = packet_buffer_[i_packet].framenum;
  frame_metadata.daq_rec = (uint64_t) packet_buffer_[i_packet].debug;
  frame_metadata.module_id = (int64_t) module_id_;
}

template<typename T, size_t N_RECV_MSG>
inline void FrameUdpReceiver<T,N_RECV_MSG>::copy_packet_to_buffers(
    ModuleFrame& metadata, char* frame_buffer, const int i_packet)
{
  size_t frame_buffer_offset = DATA_BYTES_PER_PACKET * packet_buffer_[i_packet].packetnum;
  memcpy(
      (void*) (frame_buffer + frame_buffer_offset),
      packet_buffer_[i_packet].data,
      DATA_BYTES_PER_PACKET);

  metadata.n_recv_packets++;
}

template<typename T, size_t N_RECV_MSG>
inline uint64_t FrameUdpReceiver<T,N_RECV_MSG>::process_packets(
    const int start_offset, ModuleFrame& metadata, char* frame_buffer)
{
  for (int i_packet=start_offset; i_packet < packet_buffer_n_packets_; i_packet++) {

    // First packet for this frame.
    if (metadata.pulse_id == 0) {
      init_frame(metadata, i_packet);

      // Happens if the last packet from the previous frame gets lost.
      // In the jungfrau_packet, framenum is the trigger number (how many triggers from detector power-on) happened
    } else if (metadata.frame_index != packet_buffer_[i_packet].framenum) {
      packet_buffer_loaded_ = true;
      // Continue on this packet.
      packet_buffer_offset_ = i_packet;

      return metadata.pulse_id;
    }

    copy_packet_to_buffers(metadata, frame_buffer, i_packet);

    // Last frame packet received. Frame finished.
    if (packet_buffer_[i_packet].packetnum == N_PACKETS_PER_FRAME - 1)
    {
      // Buffer is loaded only if this is not the last message.
      if (i_packet+1 != packet_buffer_n_packets_) {
        packet_buffer_loaded_ = true;
        // Continue on next packet.
        packet_buffer_offset_ = i_packet + 1;

        // If i_packet is the last packet the buffer is empty.
      } else {
        packet_buffer_loaded_ = false;
        packet_buffer_offset_ = 0;
      }

      return metadata.pulse_id;
    }
  }
  // We emptied the buffer.
  packet_buffer_loaded_ = false;
  packet_buffer_offset_ = 0;

  return 0;
}

template<typename T, size_t N_RECV_MSG>
uint64_t FrameUdpReceiver<T,N_RECV_MSG>::get_frame_from_udp(ModuleFrame& metadata, char* frame_buffer)
{
  // Reset the metadata and frame buffer for the next frame.
  metadata.pulse_id = 0;
  metadata.n_recv_packets = 0;
  memset(frame_buffer, 0, DATA_BYTES_PER_PACKET);

  // Happens when last packet from previous frame was missed.
  if (packet_buffer_loaded_) {

    auto pulse_id = process_packets(packet_buffer_offset_, metadata, frame_buffer);

    if (pulse_id != 0) {
      return pulse_id;
    }
  }

  while (true) {
    packet_buffer_n_packets_ = udp_receiver_.receive_many(msgs_, N_RECV_MSG);

    if (packet_buffer_n_packets_ == 0) {
      continue;
    }

    auto pulse_id = process_packets(0, metadata, frame_buffer);
    if (pulse_id != 0) {
      return pulse_id;
    }
  }
}