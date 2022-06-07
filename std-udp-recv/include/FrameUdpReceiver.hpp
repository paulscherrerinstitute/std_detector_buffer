#ifndef SF_DAQ_BUFFER_FRAMEUDPRECEIVER_HPP
#define SF_DAQ_BUFFER_FRAMEUDPRECEIVER_HPP

#include <netinet/in.h>
#include "PacketUdpReceiver.hpp"
#include "formats.hpp"
#include "buffer_config.hpp"

template <typename T>
class FrameUdpReceiver {
    PacketUdpReceiver udp_receiver_;
    const int n_packets_per_frame_;

    T* const packet_buffer_;
    iovec* const recv_buff_ptr_;
    mmsghdr* const msgs_;
    sockaddr_in* const sock_from_;

    bool packet_buffer_loaded_ = false;
    int packet_buffer_n_packets_ = 0;
    int packet_buffer_offset_ = 0;

    virtual void init_frame(T& frame_metadata, const int i_packet) = 0;
    void copy_packet_to_buffers(char* frame_buffer, const int i_packet);
    uint64_t process_packets(const int n_packets, T& meta, char* frame_buffer);

public:
    FrameUdpReceiver(uint16_t port, size_t n_packets_per_frame);
    virtual ~FrameUdpReceiver();
    uint64_t get_frame_from_udp(T& meta, char* frame_buffer);
};


#endif //SF_DAQ_BUFFER_FRAMEUDPRECEIVER_HPP
