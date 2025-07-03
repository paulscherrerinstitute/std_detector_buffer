#include <vector>
#include <iostream>
#include <cstring>
#include <zmq.h>

#include "std_buffer/rdma.pb.h"
#include "std_data_chunker_common/ucx_rdma_endpoint.hpp"

constexpr size_t BUFFER_SIZE = 1UL << 30;
constexpr char ZMQ_ENDPOINT[] = "tcp://*:5555";
constexpr uint32_t PROTOCOL_VERSION = 1;

int main()
{
  std::vector<std::byte> buffer(BUFFER_SIZE);

  const char* hello = "hello world";
  std::memcpy(buffer.data(), hello, std::strlen(hello));

  auto endpoint = sdcc::ucx_rdma_endpoint::create_server(buffer);

  void* zmq_ctx = zmq_ctx_new();
  void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REP);
  zmq_bind(zmq_sock, ZMQ_ENDPOINT);

  while (true) {
    zmq_msg_t zmsg;
    zmq_msg_init(&zmsg);
    int rc = zmq_msg_recv(&zmsg, zmq_sock, 0);
    if (rc < 0) continue;

    std::string req_wire(static_cast<char*>(zmq_msg_data(&zmsg)), zmq_msg_size(&zmsg));
    zmq_msg_close(&zmsg);

    ucx_ctrl::HandshakeRequest req;
    if (!req.ParseFromString(req_wire)) {
      // Send error response (protobuf)
      ucx_ctrl::ErrorResponse err;
      err.set_error_msg("Malformed handshake request");
      err.set_error_code(100);
      std::string err_wire;
      err.SerializeToString(&err_wire);
      zmq_send(zmq_sock, err_wire.data(), err_wire.size(), 0);
      continue;
    }

    // --- Send handshake response (protobuf) using data from endpoint ---
    ucx_ctrl::HandshakeResponse resp;
    // All pointers to byte data, not C strings!
    auto worker_addr = endpoint.worker_address();
    resp.set_worker_address(reinterpret_cast<const char*>(worker_addr.data()), worker_addr.size());
    resp.set_buffer_base(endpoint.buffer_base());
    resp.set_buffer_size(endpoint.buffer_size());
    auto rkey_blob = endpoint.rkey_blob();
    resp.set_rkey_blob(reinterpret_cast<const char*>(rkey_blob.data()), rkey_blob.size());
    resp.set_protocol_version(PROTOCOL_VERSION);

    std::string resp_wire;
    resp.SerializeToString(&resp_wire);
    zmq_send(zmq_sock, resp_wire.data(), resp_wire.size(), 0);

    std::cout << "[server] Sent handshake response (" << resp_wire.size() << " bytes)\n";
  }

  zmq_close(zmq_sock);
  zmq_ctx_term(zmq_ctx);
  return 0;
}
