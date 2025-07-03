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

constexpr char ZMQ_SERVER_ENDPOINT[] = "tcp://localhost:5555";
constexpr uint32_t PROTOCOL_VERSION = 1;

int main()
{
  void* zmq_ctx = zmq_ctx_new();
  void* zmq_sock = zmq_socket(zmq_ctx, ZMQ_REQ);
  zmq_connect(zmq_sock, ZMQ_SERVER_ENDPOINT);

  // --- Build and send handshake request ---
  ucx_ctrl::HandshakeRequest req;
  req.set_client_id("myclient-123");
  req.set_protocol_version(PROTOCOL_VERSION);
  std::string req_wire;
  req.SerializeToString(&req_wire);
  zmq_send(zmq_sock, req_wire.data(), req_wire.size(), 0);

  // --- Receive server reply ---
  zmq_msg_t zmsg;
  zmq_msg_init(&zmsg);
  int rc = zmq_msg_recv(&zmsg, zmq_sock, 0);
  if (rc < 0) return 1;
  std::string resp_wire(static_cast<char*>(zmq_msg_data(&zmsg)), zmq_msg_size(&zmsg));
  zmq_msg_close(&zmsg);

  // --- Try parsing both success and error response ---
  ucx_ctrl::HandshakeResponse resp;
  ucx_ctrl::ErrorResponse err;
  if (resp.ParseFromString(resp_wire)) {
    std::cout << "Connected to server!\n";
    std::cout << "Buffer base: 0x" << std::hex << resp.buffer_base() << std::dec << "\n";
    std::cout << "Buffer size: " << resp.buffer_size() << "\n";
    std::cout << "rkey_blob: " << resp.rkey_blob().size() << " bytes\n";

    const std::string& worker_addr_str = resp.worker_address();
    const uint64_t buffer_base = resp.buffer_base();
    const uint64_t buffer_size = resp.buffer_size();
    const std::string& rkey_blob = resp.rkey_blob();

    // --- 1. Initialize UCX context and worker ---
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

    // --- 2. Create endpoint to server worker using received worker address ---
    ucp_ep_params_t ep_params{};
    ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
    ep_params.address = reinterpret_cast<const ucp_address_t*>(worker_addr_str.data());
    ucp_ep_h ep = nullptr;
    if (ucp_ep_create(worker, &ep_params, &ep) != UCS_OK)
      throw std::runtime_error("ucp_ep_create failed");

    // --- 3. Import rkey for remote memory access ---
    ucp_rkey_h rkey = nullptr;
    if (ucp_ep_rkey_unpack(ep, rkey_blob.data(), &rkey) != UCS_OK)
      throw std::runtime_error("ucp_ep_rkey_unpack failed");

    // --- 4. Perform a UCX RDMA GET from remote buffer ---
    // Let's read first 4096 bytes as a demo
    size_t read_bytes = std::min<size_t>(4096, buffer_size);
    std::vector<uint8_t> local_buf(read_bytes, 0);

    ucp_request_param_t get_param{};
    bool completed = false;
    get_param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK | UCP_OP_ATTR_FIELD_USER_DATA;
    get_param.cb.send = [](void* , ucs_status_t status, void* user_data) {
      bool* completed_ptr = static_cast<bool*>(user_data);
      *completed_ptr = true;
      if (status != UCS_OK) std::cerr << "RDMA GET failed\n";
    };
    get_param.user_data = &completed;

    void* get_req = ucp_get_nbx(ep, local_buf.data(), read_bytes, buffer_base, rkey, &get_param);

    if (UCS_PTR_IS_PTR(get_req)) {
      while (!completed) {
        ucp_worker_progress(worker);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      ucp_request_free(get_req);
    }

    std::cout << "First bytes of remote buffer: ";
    for (size_t i = 0; i < std::min<size_t>(16, local_buf.size()); ++i)
      std::cout << std::hex << (int)local_buf[i] << " ";
    std::cout << std::dec << "\n";

    // --- 5. Cleanup ---
    ucp_rkey_destroy(rkey);
    ucp_ep_destroy(ep);
    ucp_worker_destroy(worker);
    ucp_cleanup(ctx);

    std::cout << "RDMA GET complete and endpoint cleaned up.\n";
  }
  else if (err.ParseFromString(resp_wire)) {
    std::cerr << "Server error: " << err.error_msg() << " code " << err.error_code() << "\n";
    return 2;
  }
  else {
    std::cerr << "Unknown reply from server\n";
    return 3;
  }

  zmq_close(zmq_sock);
  zmq_ctx_term(zmq_ctx);
  return 0;
}
