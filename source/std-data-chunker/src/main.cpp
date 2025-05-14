// server.cpp
#include <ucp/api/ucp.h>
#include <ucs/type/status.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <random>

struct peer_info_t
{
  uint64_t msg_tag;
  uint64_t ctrl_tag;
  uint64_t checksum;
};

uint64_t hash64bits(uint64_t a, uint64_t b)
{
  // Simple combination (e.g., XOR-based). Adjust as needed to match Python’s implementation.
  return a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2));
}

uint64_t generate_tag()
{
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  return dis(gen);
}

// Global container for connected client endpoints and its mutex.
std::vector<ucp_ep_h> connected_clients;
std::mutex clients_mutex;

void stream_send(ucp_ep_h ep, const void* buffer, size_t length, ucp_worker_h worker)
{
  ucp_request_param_t param = {};
  param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK;
  param.cb.send = [](void*, ucs_status_t, void*) {};
  param.request = nullptr;

  ucs_status_ptr_t request = ucp_stream_send_nbx(ep, buffer, length, &param);
  if (UCS_PTR_IS_ERR(request)) {
    throw std::runtime_error("ucp_stream_send_nbx failed");
  }
  if (UCS_PTR_STATUS(request) != UCS_OK) {
    ucp_worker_wait(worker);
    ucp_request_free(request);
  }
}

void stream_recv(ucp_ep_h ep, void* buffer, size_t length, ucp_worker_h worker)
{
  ucs_status_ptr_t request = nullptr;
  ucp_request_param_t param = {};
  param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK;
  param.cb.recv = [](void*, ucs_status_t, const ucp_tag_recv_info_t*, void*) {};
  request = ucp_stream_recv_nbx(ep, buffer, length, 0, &param);
  if (UCS_PTR_IS_ERR(request)) {
    throw std::runtime_error("ucp_stream_recv_nbx failed");
  }
  if (UCS_PTR_STATUS(request) != UCS_OK) {
    ucp_worker_wait(worker);
    ucp_request_free(request);
  }
}

void connection_handler(ucp_conn_request_h conn_request, void* arg)
{
  auto worker = static_cast<ucp_worker_h>(arg);
  ucp_ep_params_t ep_params{};
  ep_params.field_mask = UCP_EP_PARAM_FIELD_CONN_REQUEST | UCP_EP_PARAM_FIELD_ERR_HANDLER;
  ep_params.conn_request = conn_request;
  ep_params.err_handler.cb = [](void*, ucp_ep_h, ucs_status_t status) {
    std::cerr << "Endpoint error: " << ucs_status_string(status) << std::endl;
  };

  ucp_ep_h ep = nullptr;
  ucs_status_t status = ucp_ep_create(worker, &ep_params, &ep);
  if (status != UCS_OK) {
    std::cerr << "Failed to create endpoint: " << ucs_status_string(status) << std::endl;
    return;
  }
  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    connected_clients.push_back(ep);
  }
  std::cout << "New client connected." << std::endl;

  // Generate server's peer info.
  peer_info_t my_info;
  my_info.msg_tag = generate_tag();
  my_info.ctrl_tag = generate_tag();
  my_info.checksum = hash64bits(my_info.msg_tag, my_info.ctrl_tag);

  // Prepare buffers.
  char my_info_buf[sizeof(peer_info_t)];
  std::memcpy(my_info_buf, &my_info, sizeof(my_info));
  char peer_info_buf[sizeof(peer_info_t)] = {0};

  try {
    // Listener sends first then receives (matching Python protocol).
    std::cout << "Sending peer info..." << std::endl;
    stream_send(ep, my_info_buf, sizeof(my_info_buf), worker);
    std::cout << "Receiving peer info..." << std::endl;
    stream_recv(ep, peer_info_buf, sizeof(peer_info_buf), worker);
  }
  catch (const std::exception& e) {
    std::cerr << "Peer info exchange failed: " << e.what() << std::endl;
    ucp_ep_close_nb(ep, 0);
    return;
  }

  // Unpack and verify remote peer info.
  peer_info_t remote_info;
  std::memcpy(&remote_info, peer_info_buf, sizeof(peer_info_t));
  // uint64_t expected_checksum = hash64bits(remote_info.msg_tag, remote_info.ctrl_tag);
  // if (expected_checksum != remote_info.checksum) {
  //   std::cerr << "Checksum invalid in peer info exchange." << std::endl;
  //   ucp_ep_close_nb(ep, 0);
  //   return;
  // }
  std::cout << "Peer info exchange complete. Remote: msg_tag=0x" << std::hex << remote_info.msg_tag
            << ", ctrl_tag=0x" << remote_info.ctrl_tag << std::endl;
}

// Updated blocking_send() using ucp_tag_send_nbx with ucp_request_param_t.
void blocking_send(ucp_ep_h ep, const char* buffer, size_t length, ucp_worker_h worker)
{
  std::cout << "Sending." << std::endl;
  ucp_request_param_t param{};
  param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK;
  param.cb.send = [](void*, ucs_status_t, void*) {};

  // Initiate the nonblocking tagged send with tag 0.
  ucs_status_ptr_t request = ucp_tag_send_nbx(ep, buffer, length, 0, &param);
  if (UCS_PTR_IS_ERR(request)) {
    throw std::runtime_error("ucp_tag_send_nbx failed");
  }
  // If the operation completed immediately, request equals UCS_OK.
  if (UCS_PTR_STATUS(request) == UCS_OK) {
    return;
  }
  // Otherwise, poll the worker progress until the send is complete.
  while (ucp_request_check_status(request) == UCS_INPROGRESS) {
    ucp_worker_progress(worker);
  }
  ucp_request_free(request);
}

// Broadcasts a 1GB message to all connected clients.
void broadcast_message(ucp_worker_h worker)
{
  constexpr size_t msg_size = 1024ULL; // 1GB
  std::vector<char> msg(msg_size, 0);
  constexpr char greeting[13] = "hello, world";
  std::memcpy(msg.data(), greeting, sizeof(greeting));

  std::lock_guard<std::mutex> lock(clients_mutex);
  for (auto it = connected_clients.begin(); it != connected_clients.end();) {
    try {
      blocking_send(*it, msg.data(), msg.size(), worker);
      ++it;
    }
    catch (const std::exception& e) {
      std::cerr << "Error sending to client, removing endpoint: " << e.what() << std::endl;
      // Close the endpoint and remove it from the list.
      ucp_ep_close_nb(*it, 0);
      it = connected_clients.erase(it);
    }
  }
}

int main()
{
  ucp_params_t ucp_params{};
  ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES;
  ucp_params.features = UCP_FEATURE_TAG | UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;

  ucp_config_t* config = nullptr;
  ucs_status_t status = ucp_config_read(nullptr, nullptr, &config);
  if (status != UCS_OK) {
    std::cerr << "Failed to read UCX config: " << ucs_status_string(status) << std::endl;
    std::exit(EXIT_FAILURE);
  }

  ucp_context_h context = nullptr;
  status = ucp_init(&ucp_params, config, &context);
  ucp_config_release(config);
  if (status != UCS_OK) {
    std::cerr << "Failed to initialize UCX: " << ucs_status_string(status) << std::endl;
    std::exit(EXIT_FAILURE);
  }

  ucp_worker_params_t worker_params{};
  worker_params.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  worker_params.thread_mode = UCS_THREAD_MODE_SINGLE;

  ucp_worker_h worker = nullptr;
  status = ucp_worker_create(context, &worker_params, &worker);
  if (status != UCS_OK) {
    std::cerr << "Failed to create UCX worker: " << ucs_status_string(status) << std::endl;
    ucp_cleanup(context);
    std::exit(EXIT_FAILURE);
  }

  ucp_worker_h ucp_data_worker;
  status = ucp_worker_create(context, &worker_params, &ucp_data_worker);
  if (status != UCS_OK) {
    std::cerr << "Failed to create UCX worker: " << ucs_status_string(status) << std::endl;
    ucp_cleanup(context);
    std::exit(EXIT_FAILURE);
  }

  // Set up a listening socket address on port 13337.
  constexpr int SERVER_PORT = 13337;
  struct sockaddr_in listen_addr{};
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(SERVER_PORT);
  listen_addr.sin_addr.s_addr = INADDR_ANY;

  ucp_listener_params_t listener_params{};
  listener_params.field_mask =
      UCP_LISTENER_PARAM_FIELD_SOCK_ADDR | UCP_LISTENER_PARAM_FIELD_CONN_HANDLER;
  listener_params.sockaddr.addr = reinterpret_cast<struct sockaddr*>(&listen_addr);
  listener_params.sockaddr.addrlen = sizeof(listen_addr);
  listener_params.conn_handler.cb = connection_handler;
  listener_params.conn_handler.arg = worker;


  ucp_listener_h listener = nullptr;
  status = ucp_listener_create(worker, &listener_params, &listener);
  if (status != UCS_OK) {
    std::cerr << "Failed to create listener: " << ucs_status_string(status) << std::endl;
    ucp_worker_destroy(worker);
    ucp_cleanup(context);
    std::exit(EXIT_FAILURE);
  }
  std::cout << "Server listening on port " << SERVER_PORT << std::endl;

  // Main progress loop: drive UCX operations and broadcast every second.
  using clock = std::chrono::steady_clock;
  auto next_broadcast = clock::now() + std::chrono::seconds(1);
  while (true) {
    if (clock::now() >= next_broadcast) {
      ucp_worker_progress(worker);
      // broadcast_message(worker);
      next_broadcast += std::chrono::seconds(1);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Cleanup (this section is never reached in this infinite loop).
  ucp_listener_destroy(listener);
  ucp_worker_destroy(worker);
  ucp_cleanup(context);
  return 0;
}
