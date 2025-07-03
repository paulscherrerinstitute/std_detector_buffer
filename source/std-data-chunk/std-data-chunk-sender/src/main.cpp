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
  std::vector<std::byte> rdma_buffer(BUFFER_SIZE);

  const char* hello = "hello world";
  std::memcpy(rdma_buffer.data(), hello, std::strlen(hello));

  auto endpoint = sdcc::ucx_rdma_endpoint::create_server(rdma_buffer);

  void* zmq_ctx = zmq_ctx_new();
  void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REP);
  zmq_bind(zmq_sock, ZMQ_ENDPOINT);
  char buffer[512];
  ucx_ctrl::HandshakeRequest req;

  while (true) {
    if (auto n_bytes = zmq_recv(zmq_sock, buffer, sizeof(buffer), 0); n_bytes > 0) {
      if (req.ParseFromArray(buffer, n_bytes)) {
        ucx_ctrl::HandshakeResponse resp;

        auto worker_addr = endpoint.worker_address();
        resp.set_worker_address(reinterpret_cast<const char*>(worker_addr.data()),
                                worker_addr.size());
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

      else {

        ucx_ctrl::ErrorResponse err;
        err.set_error_msg("Malformed handshake request");
        err.set_error_code(100);
        std::string err_wire;
        err.SerializeToString(&err_wire);
        zmq_send(zmq_sock, err_wire.data(), err_wire.size(), 0);
      }
    }
  }

  zmq_close(zmq_sock);
  zmq_ctx_term(zmq_ctx);
  return 0;
}
