// server.cpp
#include <ucp/api/ucp.h>
#include <ucs/type/status.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

#include <zmq.h>

#include "std_buffer/rdma.pb.h"

constexpr size_t BUFFER_SIZE = 1UL << 30;
constexpr char ZMQ_ENDPOINT[] = "tcp://*:5555";
constexpr uint32_t PROTOCOL_VERSION = 1;

int main()
{
  // UCX setup (as before)
  ucp_context_h ctx = nullptr;
  ucp_worker_h worker = nullptr;
  ucp_params_t params{};
  params.field_mask = UCP_PARAM_FIELD_FEATURES;
  params.features = UCP_FEATURE_RMA;
  ucp_config_t* config = nullptr;
  if (ucp_config_read(nullptr, nullptr, &config) != UCS_OK)
    throw std::runtime_error("ucp_config_read failed");
  if (ucp_init(&params, config, &ctx) != UCS_OK) throw std::runtime_error("ucp_init failed");
  ucp_config_release(config);

  ucp_worker_params_t wparams{};
  wparams.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  wparams.thread_mode = UCS_THREAD_MODE_SINGLE;
  if (ucp_worker_create(ctx, &wparams, &worker) != UCS_OK)
    throw std::runtime_error("ucp_worker_create failed");

  // Buffer and registration
  std::vector<uint8_t> buffer(BUFFER_SIZE);
  ucp_mem_map_params_t mmap_params{};
  mmap_params.field_mask = UCP_MEM_MAP_PARAM_FIELD_ADDRESS | UCP_MEM_MAP_PARAM_FIELD_LENGTH |
                           UCP_MEM_MAP_PARAM_FIELD_FLAGS;
  mmap_params.address = buffer.data();
  mmap_params.length = buffer.size();
  mmap_params.flags = UCP_MEM_MAP_NONBLOCK;
  ucp_mem_h memh = nullptr;
  if (ucp_mem_map(ctx, &mmap_params, &memh) != UCS_OK)
    throw std::runtime_error("ucp_mem_map failed");
  // Put "hello world" at the start of the buffer
  const char* hello = "hello world";
  std::memcpy(buffer.data(), hello, std::strlen(hello));

  // rkey
  void* rkey_buf = nullptr;
  size_t rkey_size = 0;
  if (ucp_rkey_pack(ctx, memh, &rkey_buf, &rkey_size) != UCS_OK)
    throw std::runtime_error("ucp_rkey_pack failed");

  ucp_address_t* worker_addr = nullptr;
  size_t addr_len = 0;
  if (ucp_worker_get_address(worker, &worker_addr, &addr_len) != UCS_OK)
    throw std::runtime_error("ucp_worker_get_address failed");
  // ZMQ
  void* zmq_ctx = zmq_ctx_new();
  void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REP);
  zmq_bind(zmq_sock, ZMQ_ENDPOINT);

  while (true) {
    // --- Receive client request ---
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

    // Optionally, check protocol_version, client_id, etc.

    // --- Send handshake response (protobuf) ---
    ucx_ctrl::HandshakeResponse resp;
    resp.set_worker_address(reinterpret_cast<const char*>(worker_addr), addr_len);
    resp.set_buffer_base(reinterpret_cast<uint64_t>(buffer.data()));
    resp.set_buffer_size(buffer.size());
    resp.set_rkey_blob(rkey_buf, rkey_size);
    resp.set_protocol_version(PROTOCOL_VERSION);

    std::string resp_wire;
    resp.SerializeToString(&resp_wire);
    zmq_send(zmq_sock, resp_wire.data(), resp_wire.size(), 0);

    std::cout << "[server] Sent handshake response (" << resp_wire.size() << " bytes)\n";
  }

  ucp_rkey_buffer_release(rkey_buf);
  ucp_worker_release_address(worker, worker_addr);
  ucp_mem_unmap(ctx, memh);
  ucp_worker_destroy(worker);
  ucp_cleanup(ctx);
  zmq_close(zmq_sock);
  zmq_ctx_term(zmq_ctx);

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
