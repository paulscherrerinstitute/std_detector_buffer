#include <iostream>
#include <stdexcept>
#include <RingBuffer.hpp>
#include <BufferH5Writer.hpp>
#include "zmq.h"
#include "buffer_config.hpp"
#include "jungfrau.hpp"
#include "BufferUdpReceiver.hpp"

#include <sys/resource.h>
#include <syscall.h>



using namespace std;
using namespace core_buffer;

int main (int argc, char *argv[]) {
    if (argc != 5) {
        cout << endl;
        cout << "Usage: sf_buffer [device_name] [udp_port] [root_folder]";
        cout << "[source_id]";
        cout << endl;
        cout << "\tdevice_name: Name to write to disk.";
        cout << "\tudp_port: UDP port to connect to." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << "\tsource_id: ID of the source for live stream." << endl;
        cout << endl;

        exit(-1);
    }

    string device_name = string(argv[1]);
    int udp_port = atoi(argv[2]);
    string root_folder = string(argv[3]);
    int source_id = atoi(argv[4]);

    stringstream ipc_stream;
    ipc_stream << BUFFER_LIVE_IPC_URL << source_id;
    const auto ipc_address = ipc_stream.str();

    auto ctx = zmq_ctx_new();
    auto socket = zmq_socket(ctx, ZMQ_PUB);

    const int sndhwm = BUFFER_ZMQ_SNDHWM;
    if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0)
        throw runtime_error(strerror (errno));

    const int linger_ms = 0;
    if (zmq_setsockopt(socket, ZMQ_LINGER, &linger_ms, sizeof(linger_ms)) != 0)
        throw runtime_error(strerror (errno));

    if (zmq_bind(socket, ipc_address.c_str()) != 0)
        throw runtime_error(strerror (errno));

    uint64_t stats_counter(0);
    uint64_t n_missed_packets = 0;
    uint64_t n_missed_frames = 0;
    uint64_t n_corrupted_frames = 0;
    uint64_t last_pulse_id = 0;

    BufferH5Writer writer(device_name, root_folder);
    BufferUdpReceiver receiver(udp_port, source_id);

    pid_t tid;
    tid = syscall(SYS_gettid);
    int ret = setpriority(PRIO_PROCESS, tid, -20);
    if (ret == -1) throw runtime_error("cannot set nice");

    ModuleFrame metadata;
    auto frame_buffer = new char[MODULE_N_BYTES * JUNGFRAU_N_MODULES];

    while (true) {

        auto pulse_id = receiver.get_frame_from_udp(metadata, frame_buffer);

        writer.set_pulse_id(pulse_id);
        writer.write(&metadata, frame_buffer);

        zmq_send(socket, &metadata, sizeof(ModuleFrame), ZMQ_SNDMORE);
        zmq_send(socket, frame_buffer, MODULE_N_BYTES, 0);

        // TODO: Make real statistics, please.
        stats_counter++;

        if (metadata.n_received_packets < JUNGFRAU_N_PACKETS_PER_FRAME) {
            n_missed_packets +=
                    JUNGFRAU_N_PACKETS_PER_FRAME - metadata.n_received_packets;
            n_corrupted_frames++;
        }

        if (last_pulse_id>0) {
            n_missed_frames += (pulse_id - last_pulse_id) - 1;
        }
        last_pulse_id = pulse_id;

        if (stats_counter == STATS_MODULO) {
            cout << "sf_buffer:device_name " << device_name;
            cout << " sf_buffer:pulse_id " << pulse_id;
            cout << " sf_buffer:n_missed_frames " << n_missed_frames;
            cout << " sf_buffer:n_missed_packets " << n_missed_packets;
            cout << " sf_buffer:n_corrupted_frames " << n_corrupted_frames;
            cout << endl;

            stats_counter = 0;
            n_missed_packets = 0;
            n_corrupted_frames = 0;
            n_missed_frames = 0;
        }
    }

    delete[] frame_buffer;
}
